#include <stdio.h>
#include <stdlib.h>
#include "bmpr.h"

#define THRESHOLD 0.3

unsigned char toCGC[256];
unsigned char toPBC[256];
unsigned char matrix[8][8];
unsigned char secmatrix[8][8];
unsigned char bitslices[8][8][8];	//3D array to store bit slices
unsigned char secret[7][8];			// array to store bits of secret data
int add=0,add2=0;
char color, mode;
int sigbits;

unsigned char checker[8][8] =		// 8x8 checker board matrix
	{{0,1,0,1,0,1,0,1},
	 {1,0,1,0,1,0,1,0},
	 {0,1,0,1,0,1,0,1},
	 {1,0,1,0,1,0,1,0},
	 {0,1,0,1,0,1,0,1},
	 {1,0,1,0,1,0,1,0},
	 {0,1,0,1,0,1,0,1},
	 {1,0,1,0,1,0,1,0}};

unsigned char secretImageArray[7];
FILE *pSecretFile;

// function prototypes
void readCover(RGB *pixel, int width, int bottom);
int writeOut(RGB *pixel, int width, int bottom);
int readStego(RGB *pixel, int width, int bottom);
void buildGrayCode();
void getbitplanes();
unsigned char getBit(unsigned char byte, int bitloc);
double calc_complex(unsigned char array[8][8][8], int bitnum);
int writeOutFile(FILE *pFile, int pixwritten, int width);
void conjugate(unsigned char ncomp[8][8][8], int bitplane);
void set_indicator_to(int x, int bitplane);
int readIndicator(int bitplane);
void getSecretBits();
void getSecretBytes();
void getBytesBack();
void printUsage();

int main(int argc, char *argv[]){
 
	if((argc < 5)||(argc > 6)){
		printUsage();
		exit(0);
	}
	
	// set character to 'e' for extract or 'h' for hiding
	mode = argv[1][0];
	
	// set character to 'r', 'g', or 'b' for which color to use
	color = argv[2][0];

	sigbits = 8; 			// Choose number of significant bits
	
	if(argc==5){
	// extraction mode
		if((mode=='e')&&(color=='r'||color=='g'||color=='b')){
			char *fileName = argv[3]; 
			char *outFileName = argv[4];
			FILE *pFile, *pOutFile;
			BITMAPFILEHEADER bmpFileHdr;
			BITMAPINFOHEADER bmpInfoHdr;
			int i, j, totalpix, x, y;
			int bitplane;
			RGB *pixel;
			
			printf("stego-file name: %s\n",fileName);
			
			pFile = fopen(fileName,"rb");
			if (pFile == NULL) { 
				fprintf(stderr,"fopen error\n"); 
				exit(-1);
			}
	   
			pOutFile = fopen(outFileName,"wb");
			if (pOutFile == NULL) {
				fprintf(stderr,"fopen error\n"); 
				exit(1);
			}
			
			//read file header	
			fread(&bmpFileHdr, 1, sizeof(BITMAPFILEHEADER), pFile);
	   
			//read info header
			fread(&bmpInfoHdr, 1, sizeof(BITMAPINFOHEADER), pFile);
			
			// total pixels in the stego-image, width x height
			totalpix = bmpInfoHdr.biWidth * bmpInfoHdr.biHeight;
			printf("number of pixels in stego file: %d\n",totalpix);
		
			// build the toCGC and toPBC arrays
			buildGrayCode();
			
			// allocate pixel array of RGB structures 
			// RGB structures contain the 3 bytes (24-bits) of each pixel
			// assign pointer to the array, which is used to access pixel data
			pixel = (RGB *)calloc(totalpix, sizeof(RGB));

			//read in all pixels to allocated memory, in order of the BMP data
			for(i=0;i<totalpix;i++) {
				fread(&pixel[i], 1, sizeof(RGB), pFile);
			}
			
			int width = bmpInfoHdr.biWidth;
			int bottom, pixleft, sfilesize, bwritten, first;
			pixleft = totalpix;
			bottom = 0;
			first = 1;
			bwritten = 0;
			sfilesize = 1;
			
			// main loop used for extraction, processes until all pixels in stego-image
			// have been processed
			while(pixleft>0){
				
				// loads next 8x8 region into matrix
				// returns bottom of next 8x8 region to be processed
				bottom = readStego(pixel, width, bottom);
				
				// convert PBC values to CGC
				for(i=0;i<8;i++){
					for(j=0;j<8;j++){
						matrix[i][j] = toCGC[matrix[i][j]];
					}
				}
				
				// slice current 8x8 region into global bitslices array
				getbitplanes();
				

				// process as many bit planes as desired for the current region
				for(bitplane = 8-sigbits; bitplane < 8; bitplane++){
					
					if (calc_complex(bitslices, bitplane) >= THRESHOLD){
					// if current bitplane of the current region is complex past the threshold
					// extract data
					
						if(readIndicator(bitplane)==1){
							// if conjugation indicator is set
							// conjugate to get original data
							conjugate(bitslices, bitplane);
						}
						
						// load current bitplane into secret matrix for processing
						for(x=0;x<7;x++){
							for(y=0;y<8;y++){
								secret[x][y] = bitslices[x][y][bitplane];
							}
						}
						
						// puts bits from secret into bytes
						getSecretBytes();
						
						if(first == 1){
							// if first block, header for bitmap
							// gives filesize of secret image
							printf("type of secret file: %c%c\n",secretImageArray[0], secretImageArray[1]);
							sfilesize = (secretImageArray[5] << 24)+ (secretImageArray[4] << 16)+ (secretImageArray[3]<<8)+secretImageArray[2];
							printf("size of secret file: %d bytes\n",sfilesize);
							first = 0;
						}
				
						for(i=0;i < 7;i++){
							// write to output file if secret file size has not been met
							if(!(bwritten<sfilesize))
								break;
							bwritten += fwrite(&secretImageArray[i], 1, 1, pOutFile);
						}
					}
				}
				
				//exit loop if secret file size has been reached
				if(bwritten==sfilesize)
					break;
					
				// have processed 64 pixels (8x8 region)
				pixleft -= 64;
			}
			

			fclose(pFile);
			fclose(pOutFile);	
			free(pixel);			
			
		}else{
			printUsage();
			exit(0);
		}
	}
	if(argc==6){
	// hiding mode
	// most of algorithm is the same as extraction
		if((mode=='h')&&(color=='r'||color=='g'||color=='b')){
			char *fileName = argv[3]; 
			char *secretFileName = argv[4];
			char *outFileName = argv[5];
			FILE *pFile, *pOutFile;
			BITMAPFILEHEADER bmpFileHdr;
			BITMAPINFOHEADER bmpInfoHdr;
			int i, j, totalpix, x, y;
			int bitplane;
			
			RGB *pixel;
		
			printf("cover file name: %s\n",fileName);
			printf("secret file name: %s\n",secretFileName);
			
			pSecretFile = fopen(secretFileName,"rb");
			if (pSecretFile == NULL) {
				fprintf(stderr,"fopen error\n"); 
				exit(-1);
			}
			
			pFile = fopen(fileName,"rb");
			if (pFile == NULL) { 
				fprintf(stderr,"fopen error\n"); 
				exit(-1);
			}
	   
			pOutFile = fopen(outFileName,"wb");
			if (pOutFile == NULL) {
				fprintf(stderr,"fopen error\n"); 
				exit(1);
			}
	   
	   

			//read file header	
			fread(&bmpFileHdr, 1, sizeof(BITMAPFILEHEADER), pFile);
			if(bmpFileHdr.bfType != 0x4d42) {
				fprintf(stderr,"Not BM\n");
			}
			//write file header to output file
			fwrite(&bmpFileHdr,1, sizeof(BITMAPFILEHEADER), pOutFile);
	   
			//read info header
			fread(&bmpInfoHdr, 1, sizeof(BITMAPINFOHEADER), pFile);
	   
			//write info header to output file
			fwrite(&bmpInfoHdr,1, sizeof(BITMAPINFOHEADER), pOutFile);
		
			totalpix = bmpInfoHdr.biWidth * bmpInfoHdr.biHeight;
			printf("number of pixels in cover file: %d\n",totalpix);
		
			buildGrayCode();

			pixel = (RGB *)calloc(totalpix, sizeof(RGB));

			for(i=0;i<totalpix;i++) {
				fread(&pixel[i], 1, sizeof(RGB), pFile);
			}

			int width = bmpInfoHdr.biWidth;
			int bottom, pixleft;
			pixleft = totalpix;
			bottom = 0;
			while(pixleft>0){
				readCover(pixel, width, bottom);
				for(i=0;i<8;i++){
					for(j=0;j<8;j++){
						matrix[i][j] = toCGC[matrix[i][j]];
					}
				}

				getbitplanes();
						
				for(bitplane = 8-sigbits; bitplane < 8 ; bitplane++){
					if (calc_complex(bitslices, bitplane) >= THRESHOLD){
				
					 
						fread(&secretImageArray, 1, 7, pSecretFile);
					
						getSecretBits();
						for(x=0;x<7;x++){
							for(y=0;y<8;y++){
								bitslices[x][y][bitplane] = secret[x][y];
							}
					
						}
						if( calc_complex(bitslices,bitplane) < THRESHOLD ){
							conjugate(bitslices, bitplane);
						}else{
							set_indicator_to(0, bitplane);
						}

					}
				}

				getBytesBack();
			
				for(i=0;i<8;i++){
					for(j=0;j<8;j++){
						matrix[i][j] = toPBC[matrix[i][j]];
					}
				}
			
				bottom = writeOut(pixel, width, bottom);

				pixleft -= 64;
			}
			for(i=0;i<totalpix;i++) {
				fwrite(&pixel[i], 1, sizeof(RGB), pOutFile);
			}

			fclose(pFile);
			fclose(pOutFile);
			fclose(pSecretFile);
			free(pixel);
		
		}else{
			printUsage();
			exit(0);
		}			
	}
	
	exit(0);
}

// loads next 8x8 region into matrix for processing
// uses width to determine how many bytes to skip to reach next row
// uses bottom to keep track of where the bottom of current region is
// used only for hiding
void readCover(RGB *pixel, int width, int bottom){
	int top, i, j, k;
		
	// calculates 8 rows above the bottom
	top = bottom+(width*7)+8;
	k = 0;

	for(i=bottom;i<top;i+=width){
		for(j=0; j<8; j++){
			if(color=='r')
				matrix[k][j] = pixel[i+j].Red;
			if(color=='g')
				matrix[k][j] = pixel[i+j].Green;
			if(color=='b')
				matrix[k][j] = pixel[i+j].Blue;
		}
		k++;
	}

}

// loads working matrix values into actual pixel array
// uses bottom to set where next region bottom is
// used only for hiding
int writeOut(RGB *pixel, int width, int bottom){
	int top, i, j, k;
	static int pixread = 0;	
	top = bottom+(width*7)+8;
	k = 0;

	for(i=bottom;i<top;i+=width){
		for(j=0; j<8; j++){
			if(color=='r')
				pixel[i+j].Red = matrix[k][j];
			if(color=='g')
				pixel[i+j].Green = matrix[k][j];
			if(color=='b')
				pixel[i+j].Blue = matrix[k][j];
		}
		k++;
	}
	pixread += 64;
	bottom += 8;
	if((pixread % (width * 8)) == 0){
		bottom += (width * 7);
	}
	return bottom;
}


// used only for extraction
// reads through stego-file
// loads 8x8 regions from stego-file into matrix for use
int readStego(RGB *pixel, int width, int bottom){
	int top, i, j, k;
	static int pixread = 0;	
	top = bottom+(width*7)+8;
	k = 0;

	for(i=bottom;i<top;i+=width){
		for(j=0; j<8; j++){
			if(color=='r')
				matrix[k][j] = pixel[i+j].Red;
			if(color=='g')
				matrix[k][j] = pixel[i+j].Green;
			if(color=='b')
				matrix[k][j] = pixel[i+j].Blue;
		}
		k++;
	}
	pixread += 64;
	bottom += 8;
	
	// jumps to next row when end is reached
	if((pixread % (width * 8)) == 0){
		bottom += (width * 7);
	}
	return bottom;
}

// sets bit indicator on a 8x8 matrix
// at the bottom left bit, 1 if exists conjugation
// 0 otherwise
void set_indicator_to(int x, int bitplane){
   bitslices[7][0][bitplane] = x;
}

int readIndicator(int bitplane){
   return bitslices[7][0][bitplane];
}

// xor's an uncomplex matrix with a checker board matrix
// to make it complex and sets bit indicator to 1
void conjugate(unsigned char ncomp[8][8][8], int bitplane){
	int i, j;
  
	for(i=0;i<8;i++){
		for(j=0;j<8;j++){
			ncomp[i][j][bitplane] = checker[i][j] ^ ncomp[i][j][bitplane];
		}
	}
	set_indicator_to(1, bitplane);
}
// this function builds the canonical gray code array variables
// code taken from the bit map reader program provided by the Instructor
void buildGrayCode(){
        int i, length, posUp, posDw, cnt;

        length = 1;
        toCGC[0] = 0;
        toPBC[0] = 0;
        cnt = 1;

        while(length < 256){
                posUp = length - 1;
                posDw = length;
                for(i = 0; i < length; i++){
                        toCGC[i + posDw] = toCGC[posUp - i] + length;
                        toPBC[toCGC[i + posDw]] = cnt++;
                }
                length = length * 2;
        }
        return;
} // buildGrayCode

//gets the bit at the location 'bitloc'(0-7) from a byte
unsigned char getBit(unsigned char byte, int bitloc){
   unsigned char ret;

   switch(bitloc){
	   case 0:
		   ret = (byte & 0x80)>>7;break;
	   case 1:
		   ret = (byte & 0x40)>>6;break;
	   case 2:
		   ret = (byte & 0x20)>>5;break;
	   case 3:
		   ret = (byte & 0x10)>>4;break;
	   case 4:
		   ret = (byte & 0x08)>>3;break;
	   case 5:
		   ret = (byte & 0x04)>>2;break;
	   case 6:
		   ret = (byte & 0x02)>>1;break;
	   case 7:
		   ret = byte & 0x01;break;
	   default:
		   break;
   }
   return ret;
}

//slices the bit planes of an 8x8 pixel block
void getbitplanes(){
   int x, y, z;
   for(x=0;x<8;x++){
      for(y=0;y<8;y++){
         for(z=0;z<8;z++){
            bitslices[x][y][z] = getBit(matrix[x][y],z);
         }
      }
   }
}

// calculates complexity of bitplane number 'bitnum'
double calc_complex(unsigned char array[8][8][8], int bitnum){
	int x, y, sum = 0;
	for (x = 0; x < 8; x++){
		for (y = 0; y < 8; y++){
			if (y != 7)
				if (array[x][y][bitnum] != array[x][y+1][bitnum])
					sum++;
			if (x != 7)
				if (array[x][y][bitnum] != array[x+1][y][bitnum])
					sum++;
		}
	}
	return sum / 112.0;
}

// gets the bytes from the bit planes back to matrix
void getBytesBack(){
   int x, y, z;
   unsigned char toMatrix;
	for(x=0;x<8;x++){
		for(y=0;y<8;y++){
			toMatrix = 0;
			for(z=0;z<8;z++){
				switch(z){	
					case 0:
						toMatrix += bitslices[x][y][z]<<7;break;
					case 1:
						toMatrix += bitslices[x][y][z]<<6;break;
					case 2:
						toMatrix += bitslices[x][y][z]<<5;break;
					case 3:
						toMatrix += bitslices[x][y][z]<<4;break;
					case 4:
						toMatrix += bitslices[x][y][z]<<3;break;
					case 5:
						toMatrix += bitslices[x][y][z]<<2;break;
					case 6:
						toMatrix += bitslices[x][y][z]<<1;break;
					case 7:
						toMatrix += bitslices[x][y][z];break;
					default: break;
				}
			}
			matrix[x][y] = toMatrix;	
		}
	}
}

// gets the bytes from the secret matrix to secret array
void getSecretBytes(){
   int x, y;
   unsigned char toArray;
	for(x=0;x<7;x++){
		toArray = 0;
		for(y=0;y<8;y++){
			switch(y){	
				case 0:
					toArray += secret[x][y]<<7;break;
				case 1:
					toArray += secret[x][y]<<6;break;
				case 2:
					toArray += secret[x][y]<<5;break;
				case 3:
					toArray += secret[x][y]<<4;break;
				case 4:
					toArray += secret[x][y]<<3;break;
				case 5:
					toArray += secret[x][y]<<2;break;
				case 6:
					toArray += secret[x][y]<<1;break;
				case 7:
					toArray += secret[x][y];break;
				default: break;
			}
		}
		secretImageArray[x] = toArray;
	}
}

void getSecretBits() {
	int x, y;
	for(x=0;x<7;x++){
    	for(y=0;y<8;y++){
            secret[x][y] = getBit(secretImageArray[x],y);
		}
 	}
}

void printUsage(){
	printf("\nUsage for hiding: color_BPCS 'h' '[r|g|b]' 'infile' 'secretfile' 'stegofile'\n\n");
	printf("Usage for extracting: color_BPCS 'e' '[r|g|b]' 'stegofile' 'outfile'\n\n");
	printf("Where:\t'h' is hiding mode.\n");
	printf("\t'e' is extraction mode.\n");
	printf("\t'infile' is the name of the bitmap file to hide information. \n");
	printf("\t'secretfile' is the name of the secret bitmap file to hide.\n");
	printf("\t'stegofile' is the name of the file with concealed information.\n");
	printf("\t'outfile' is the name of the file to be extracted.\n");
	printf("\t'r|g|b' is where the color will be chosen to hide or extract.\n");
	printf("\t'r' for red 'g' for green 'b' for blue.\n");
}
