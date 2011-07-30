#include <stdio.h>
#include <stdlib.h>
#include "bmpr.h"

unsigned char toCGC[256];
unsigned char toPBC[256];
unsigned char matrix[8][8];
unsigned char bitslices[8][8][8];	//3D array to store bit slices
int add = 0;
unsigned char checker[8][8] =		// 8x8 checker board matrix
	{{0,1,0,1,0,1,0,1},
         {1,0,1,0,1,0,1,0},
         {0,1,0,1,0,1,0,1},
         {1,0,1,0,1,0,1,0},
         {0,1,0,1,0,1,0,1},
         {1,0,1,0,1,0,1,0},
         {0,1,0,1,0,1,0,1},
         {1,0,1,0,1,0,1,0}};



// function prototypes
int readCover(FILE *pFile, int pixread, int width, char rgb);
void buildGrayCode();
void getbitplanes();
unsigned char getBit(unsigned char byte, int bitloc);
double calc_complex(unsigned char array[8][8][8], int bitnum);

int main(int argc, char *argv[]) {
	char *fileName = argv[1];
	FILE * pFile;
	BITMAPFILEHEADER bmpFileHdr;
	BITMAPINFOHEADER bmpInfoHdr;
	int i, j;
	
	printf("file name: %s\n",fileName);
	
	pFile = fopen(fileName,"rb");
	if (pFile == NULL) { 
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

       int totalpix = bmpInfoHdr.biWidth * bmpInfoHdr.biHeight;
       int pixread=0;

       while(pixread < totalpix){
   //change this value ^
   //to store the block in the matrix
   //to get block "n" multiply n*64
          pixread = readCover(pFile,pixread,bmpInfoHdr.biWidth,'b');
       }
       for(i=0;i<8;i++){
          for(j=0;j<8;j++){
             printf("%d ", matrix[i][j]);
	  }
          printf("\n");
       }
       printf("block #%d\npixels left: %d\n",(pixread/64),(totalpix-pixread));

	buildGrayCode();
       printf("CGC values:\n");
       for(i=0;i<8;i++){
          for(j=0;j<8;j++){
	     matrix[i][j] = toCGC[matrix[i][j]];
             printf("%d ", matrix[i][j]);
	  }
          printf("\n");
       }
	getbitplanes();
	int x, y, z;
	for(z=0;z<8;z++){
	   for(x=0;x<8;x++){
	      for(y=0;y<8;y++){
	         printf("%d ", bitslices[x][y][z]);
	      }
	      printf("\n");
      	   }
           printf("complexity: %f\n",calc_complex(bitslices, z));
   	}
	

        return 0;
}

//reads cover image, returns the number of pixels left in the image
//select color to be used for hidding 'r' for red, 'g' for green, 'b' for blue
int readCover(FILE *pFile, int pixread, int width, char rgb){
   RGB color;
   int level, i, offset;
   
   for(level=0;level<8;level++){
      for(i=0;i<8;i++){
         fread(&color, 1, sizeof(RGB), pFile);
	 switch(rgb){
	    case 'r':
               matrix[level][i] = color.Red;break;
            case 'g':
	       matrix[level][i] = color.Green;break;
            case 'b':
               matrix[level][i] = color.Blue;break;
	    default:
	       printf("Wrong color\n");
	       exit(-1);
	 }

      }
      if(level<7) 
         fseek(pFile, (sizeof(RGB)*(width - 8)), SEEK_CUR);
   }
   pixread+=64;
   offset = (54 + (sizeof(RGB)*(pixread/8)));
   if((pixread % (width*8))==0){
      add += sizeof(RGB) * width * 7;
   }
   fseek(pFile, (offset+add), SEEK_SET);
   return pixread;
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






