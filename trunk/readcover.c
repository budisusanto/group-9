#include <stdio.h>
#include <stdlib.h>
#include "bmpr.h"

unsigned char matrix[8][8];
int add = 0;

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
             printf("%x ", matrix[i][j]);
	  }
          printf("\n");
       }
       printf("block #%d\npixels left: %d\n",(pixread/64),(totalpix-pixread));

        return 0;
}







