#include <stdio.h>
#include <stdlib.h>
#include "bmpr.h"

#define THRESHOLD 0.3

unsigned char toCGC[256];
unsigned char toPBC[256];
unsigned char matrixB[8][8];
unsigned char matrixG[8][8];
unsigned char matrixR[8][8];
unsigned char secmatrix[8][8];
unsigned char bitslices[8][8][8];	//3D array to store bit slices
int add=0,add2=0;

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
int readCover(FILE *pFile, int pixread, int width);
void buildGrayCode();
void getbitplanes();
unsigned char getBit(unsigned char byte, int bitloc);
double calc_complex(unsigned char array[8][8][8], int bitnum);
void readSecretImage();
int writeOutFile(FILE *pFile, int pixwritten, int width);

// sets bit indicator on a 8x8 matrix
// at the bottom left bit, 1 if exists conjugation
// 0 otherwise
void set_indicator_to(int x, int bitplane){
   matrixB[7][0] |= x << bitplane;
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
				matrixB[i][j] &= ~(1 << bitplane);
			else
				matrixB[i][j] |= 1 << bitplane;
      }
   }
   
   set_indicator_to(1, bitplane);
}

int main(int argc, char *argv[]) {
	char *fileName = argv[1]; 
	char *secretFileName = argv[2];
	char *outFileName = argv[3];
	FILE *pFile, *pOutFile;
	BITMAPFILEHEADER bmpFileHdr;
	BITMAPINFOHEADER bmpInfoHdr;
	int i, j, pixread, pixwritten, totalpix, x, y, z;
	int bit, numBytes, bitplane, r;
	
	printf("file name: %s\n",fileName);
	printf("secret file name: %s\n",secretFileName);
	
	pSecretFile = fopen(secretFileName,"rb");
	if (pSecretFile == NULL) {
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
   //write file header to output file
   fwrite(&bmpFileHdr,1, sizeof(BITMAPFILEHEADER), pOutFile);
   
   //read info header
   fread(&bmpInfoHdr, 1, sizeof(BITMAPINFOHEADER), pFile);
   
   //write info header to output file
	fwrite(&bmpInfoHdr,1, sizeof(BITMAPINFOHEADER), pOutFile);
	
	totalpix = bmpInfoHdr.biWidth * bmpInfoHdr.biHeight;
	printf("number of pixels in file: %d\n",totalpix);
	
	buildGrayCode();
	pixread = 0;
r=0;
   while(pixread < totalpix)	{
	   //change this value ^
		//to store the block in the matrix
	   //to get block "n" multiply n*64
		pixread = readCover(pFile,pixread,bmpInfoHdr.biWidth);
	
	if( r == 0) {	
		for(i=0;i<8;i++) {
			for(j=0;j<8;j++) {
				printf("%d ",matrixB[i][j]);
  		   }
			printf("\n");
		}
	}
	
		for(i=0;i<8;i++)
		{
			for(j=0;j<8;j++)
			{
				matrixB[i][j] = toCGC[matrixB[i][j]];
  		   }
  		}
	/*	
  		numBytes = fread(&secretImageArray, 1, 7, pSecretFile);
  		//printf("read secret bytes: %d\n",numBytes);
  		
		getbitplanes();
		for(bitplane = 0; bitplane <8; bitplane++)
		{
  			if (calc_complex(bitslices, bitplane) >= THRESHOLD)
  			{
  			
  				for(x=0;x<7;x++)
				{
					for(y=0;y<8;y++)
					{
		  				bit = (int) getBit(secretImageArray[x], y);
		  				if (bit == 0) 
		  					matrixB[x][y] &= ~(1 << bitplane);
		  				else
		  					matrixB[x][y] |= 1 << bitplane;
		  			}
		  		}
		  		
		  		getbitplanes();
		  		if( calc_complex(bitslices,bitplane) < THRESHOLD )
		  		{
		  			conjugate(bitslices, bitplane);
		  		}
		  		else
		  		{
		  			set_indicator_to(0, bitplane);
		  		}
  			}
		}
		*/
		for(i=0;i<8;i++)
		{
			for(j=0;j<8;j++)
			{
				matrixB[i][j] = toPBC[matrixB[i][j]];
  		   }
  		}
  		
	if(r ==0)
	{
		
			printf("\n");
		
			for(i=0;i<8;i++) {
			for(j=0;j<8;j++) {
				printf("%d ",matrixB[i][j]);
  		   }
			printf("\n");
		}
	}
	r++;
		pixwritten = writeOutFile(pOutFile, pixread, bmpInfoHdr.biWidth);
		//r++;
		//if( r == 5);
		//	break;
	}
	
	exit(0);
	for(i=0;i<8;i++) {
		for(j=0;j<8;j++) {
			printf("%d ", matrixB[i][j]);
		}

      printf("\n");
	}
   printf("block #%d\npixels left: %d\n",(pixread/64),(totalpix-pixread));

	
   printf("CGC values:\n");

	for(z=0;z<8;z++){
	   for(x=0;x<8;x++){
	      for(y=0;y<8;y++){
	         //printf("%d ", bitslices[x][y][z]);
	      }
	      //printf("\n");
      }
      //printf("complexity: %f\n",calc_complex(bitslices, z));
   }
	

   return 0;
}

//reads cover image, returns the number of pixels left in the image
//stores each color information of the pixel in 3 different matrices
int readCover(FILE *pFile, int pixread, int width){
   RGB color;
   int level, i, offset;
   
   for(level=0;level<8;level++) {
      for(i=0;i<8;i++) {
         fread(&color, 1, sizeof(RGB), pFile);
			matrixR[level][i] = color.Red;
			matrixG[level][i] = color.Green;
			matrixB[level][i] = color.Blue;
		}
      if(level<7) 
         fseek(pFile, (sizeof(RGB)*(width - 8)), SEEK_CUR);
   }
   pixread += 64;
   offset = (54 + ( sizeof(RGB) * (pixread/8)) );
   if((pixread % (width*8))==0){
      add += sizeof(RGB) * width * 7;
   }
   fseek(pFile, (offset+add), SEEK_SET);
   return pixread;
}

int writeOutFile(FILE *pFile, int pixwritten, int width){
   RGB color;
   int level, i, offset, add2;
   static int regions = 0;
   int regionsPerWidth;
   
   regionsPerWidth = width / 8;
   
   for(level=0;level<8;level++) {
      for(i=0;i<8;i++) {
         fwrite(&matrixG[level][i], 1, 1, pFile);
         fwrite(&matrixR[level][i], 1, 1, pFile);
         fwrite(&matrixB[level][i], 1, 1, pFile);
		}
      fseek(pFile, (3* (width - 8)), SEEK_CUR);
   }
   regions++;
   offset = (54 + (regions * 24));
    if((regions % regionsPerWidth) == 0){
   // 	printf("hit border\n");
    //	exit(0);
      add2 += (7 * 3 * width );
   }
   fseek(pFile, (offset+add2), SEEK_SET);
   return pixwritten;
}

int writeOutFile2(FILE *pFile, int pixwritten, int width){
   RGB color;
   int level, i, offset;
   
   for(level=0;level<8;level++) {
      for(i=0;i<8;i++) {
         fwrite(&matrixB[level][i], 1, 1, pFile);
         fwrite(&matrixG[level][i], 1, 1, pFile);
         fwrite(&matrixR[level][i], 1, 1, pFile);
		}
      if(level<7) 
         fseek(pFile, (3*(width - 8)), SEEK_CUR);
   }
   pixwritten += 64;
   offset = (54 + ( sizeof(RGB) * (pixwritten/8)) );
   if((pixwritten % (width*8))==0){
      add2 += sizeof(RGB) * width * 7;
   }
   fseek(pFile, (offset+add2), SEEK_SET);
   return pixwritten;
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
            bitslices[x][y][z] = getBit(matrixB[x][y],z);
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

