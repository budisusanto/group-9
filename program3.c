#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpr.h"

#define THRESHOLD 0.5

unsigned char toCGC[256];
unsigned char toPBC[256];
unsigned char matrix[8][8];
unsigned char secmatrix[8][8];
unsigned char bitslices[8][8][8];	//3D array to store bit slices
int add=0,add2=0;
//RGB matrix[8][8];

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
int readCover(RGB *pixel, int width, int bottom);
int writeOut(RGB *pixel, int width, int bottom);
void buildGrayCode();
void getbitplanes();
unsigned char getBit(unsigned char byte, int bitloc);
double calc_complex(unsigned char array[8][8][8], int bitnum);
void readSecretImage();
int writeOutFile(FILE *pFile, int pixwritten, int width);
void conjugate(unsigned char ncomp[8][8][8], int bitplane);
void set_indicator_to(int x, int bitplane);
void print_matrix();
int loadMatrix(int region);

int main(int argc, char *argv[]) {
	char *fileName; 
	char *secretFileName;
	char *outFileName;
	FILE *pFile, *pOutFile;
	BITMAPFILEHEADER bmpFileHdr;
	BITMAPINFOHEADER bmpInfoHdr;
	int i, j, pixread, pixwritten, totalpix, x, y, z;
	int bit, numBytes, bitplane, r, width, height, mode;
	RGB *pixel;
	RGB *pixArray;
	int regions, region_index, x_offset, y_offset, region_width, region_height;

	if (strcmp(argv[1],"-h") == 0) {
		fileName = argv[2]; 
		secretFileName = argv[3];
		outFileName = argv[4];
		mode = 1;
	}
	else if (strcmp(argv[1],"-x") == 0) {
		fileName = argv[2];
		outFileName = argv[3];
		mode = 2;
	}
	else {
		printf("usage: %s -h coverImage secretImage outputFile\n",argv[0]);
		printf("or\n");
		printf("usage: %s -x stegoFile outputFile\n",argv[0]);
	}
	
	
	printf("file name: %s\n",fileName);
	printf("secret file name: %s\n",secretFileName);
	
	pSecretFile = fopen(secretFileName,"rb");
	if (pSecretFile == NULL && mode != 2) {
		fprintf(stderr,"fopen error\n"); 
		exit(1);
	}
	pFile = fopen(fileName,"rb");
	if (pFile == NULL) {
		fprintf(stderr,"fopen error\n");
		exit(1);
	}
	pOutFile = fopen(outFileName,"wb");
	if (pOutFile == NULL) {
		fprintf(stderr,"fopen error\n"); 
		exit(1);
	}
	
	//read file header	
	fread(&bmpFileHdr, 1, sizeof(BITMAPFILEHEADER), pFile);
	if (bmpFileHdr.bfType != 0x4d42) {
		fprintf(stderr,"Not BM\n");
	}
	//read info header
	fread(&bmpInfoHdr, 1, sizeof(BITMAPINFOHEADER), pFile);
	
	buildGrayCode();

	totalpix = bmpInfoHdr.biWidth * bmpInfoHdr.biHeight;
	printf("number of pixels in file: %d\n",totalpix);	
	pixel = (RGB *) calloc(totalpix, sizeof(RGB));
	
	/*for(i=0; i < totalpix; i++) {
		fread(&pixel[i], 1, sizeof(RGB), pFile);
	}*/

	numBytes = fread(pixel, sizeof(RGB), totalpix, pFile);
	if (numBytes != totalpix) {
		perror("fread");
		exit(1);
	}
	
	width = bmpInfoHdr.biWidth;
	height = bmpInfoHdr.biHeight;
	region_width = 8;
	region_height = 8;
	region_index = 0;
	regions = totalpix / (region_width * region_height);
	printf("number of 8x8 regions in file: %d\n", regions);
	printf("width: %d\n", width);
	printf("height: %d\n", height);
	x_offset = 0;
	y_offset = 0;
	
	if (mode == 2) {
		printf("extract mode\n");
		
		fwrite(&bmpFileHdr, 1, sizeof(BITMAPFILEHEADER), pOutFile);   
		fwrite(&bmpInfoHdr, 1, sizeof(BITMAPINFOHEADER), pOutFile);
		
		while (region_index < regions) {
	
		for(x = 0; x < 8; x++) {
			for(y = 0; y < 8; y++) {
				matrix[x][y] = pixel[(region_index * region_width) + y + (x * width)].Blue;
			}
		}

		for(x = 0; x < 8; x++) {
			for(y = 0; y < 8; y++) {
				matrix[x][y] = toCGC[matrix[x][y]];
			}
		}
		
		getbitplanes();

		for(bitplane = 0; bitplane < 8; bitplane++) {
			if (calc_complex(bitslices, bitplane) >= THRESHOLD) {
				if( bitslices[7][0][bitplane] == 1 ) {
					conjugate(bitslices, bitplane);
				}
				
				for(x = 0; x < 8; x++) {
					fwrite(&(bitslices[x][0][bitplane]), 1, 1, pOutFile);
				}
				
			}
		}
		
		for(x = 0; x < 8; x++) {
			for(y = 0; y < 8; y++) {
				matrix[x][y] = toPBC[matrix[x][y]];
			}
		}
		
		for(x = 0; x < 8; x++) {
			for(y = 0; y < 8; y++) {
				pixel[(region_index * region_width) + y + (x * width)].Blue = matrix[x][y];
			}
		}
		
		region_index++;
        //writeOut(pixel, width, bottom);

		//pixleft -= 64;
	}
		
	
	}
	
	while (region_index < regions) {
	
		for(x = 0; x < 8; x++) {
			for(y = 0; y < 8; y++) {
				matrix[x][y] = pixel[(region_index * region_width) + y + (x * width)].Blue;
			}
		}

		for(x = 0; x < 8; x++) {
			for(y = 0; y < 8; y++) {
				matrix[x][y] = toCGC[matrix[x][y]];
			}
		}
		
		numBytes = fread(&secretImageArray, 1, 7, pSecretFile);
		//printf("read secret bytes: %d\n",numBytes);
  
		getbitplanes();

		for(bitplane = 0; (bitplane < 8) && (numBytes > 0); bitplane++) {
			if (calc_complex(bitslices, bitplane) >= THRESHOLD){
				for(x=0;x<7;x++){
					for(y=0;y<8;y++){
						bit = (int) getBit(secretImageArray[x], y);
						if (bit == 0) 
							matrix[x][y] &= ~(1 << bitplane);
						else
							matrix[x][y] |= 1 << bitplane;
					}
				}
		
		   		getbitplanes();
				if( calc_complex(bitslices,bitplane) < THRESHOLD ) {
					conjugate(bitslices, bitplane);
				}
				else {
					set_indicator_to(0, bitplane);
				}
			}
		} 
		
		for(x = 0; x < 8; x++) {
			for(y = 0; y < 8; y++) {
				matrix[x][y] = toPBC[matrix[x][y]];
			}
		}
		
		for(x = 0; x < 8; x++) {
			for(y = 0; y < 8; y++) {
				pixel[(region_index * region_width) + y + (x * width)].Blue = matrix[x][y];
			}
		}
		
		region_index++;
        //writeOut(pixel, width, bottom);

		//pixleft -= 64;
	}
	

	fwrite(&bmpFileHdr, 1, sizeof(BITMAPFILEHEADER), pOutFile);   
	fwrite(&bmpInfoHdr, 1, sizeof(BITMAPINFOHEADER), pOutFile);
	numBytes = fwrite(pixel, sizeof(RGB), totalpix, pOutFile);
	if (numBytes != totalpix) {
		perror("fwrite");
		exit(1);
	}

    fclose(pFile);
    fclose(pOutFile);
    fclose(pSecretFile);
	exit(0);
}

//reads cover image, returns the number of pixels left in the image
//stores each color information of the pixel in 3 different matrices
int readCover(RGB *pixel, int width, int bottom) {
	int top, i, j, k;
	static int pixread = 0;	
	top = bottom + (width * 7) + 8;
	k = 0;

	for(i = bottom; i < top; i += width) {
		for(j = 0; j < 8; j++) {
			matrix[k][j] = pixel[i+j].Blue;
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

int writeOut(RGB *pixel, int width, int bottom){
	int top, i, j, k;
	static int pixread = 0;	
	top = bottom+(width*7)+8;
	k = 0;

	for(i=bottom;i<top;i+=width){
		for(j=0; j<8; j++){
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

// sets bit indicator on a 8x8 matrix
// at the bottom left bit, 1 if exists conjugation
// 0 otherwise
void set_indicator_to(int x, int bitplane){
   matrix[7][0] |= x << bitplane;
}

// xor's an uncomplex matrix with a checker board matrix
// to make it complex and sets bit indicator to 1
void conjugate(unsigned char ncomp[8][8][8], int bitplane){
   int i, j;
   char bit;
   for(i=0;i<8;i++){
      for(j=0;j<8;j++){
         bit = checker[i][j] ^ ncomp[i][j][bitplane];
         if (bit == 0) 
				matrix[i][j] &= ~(1 << bitplane);
			else
				matrix[i][j] |= 1 << bitplane;
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

void readSecretImage() {
	int numBytes;

	
	//printf("secret file num bytes: %d\n", numBytes);
	//printf("first byte of secret file: %c\n", secretByteArray[0][0]); 

}

void print_matrix() {
	int x, y;
	printf("Current matrix of Blue pixel values\n");
	for (x = 0; x < 8; x++){
		for (y = 0; y < 8; y++){
			printf("%d ", matrix[x][y]);
		}
		printf("\n");
	}
}