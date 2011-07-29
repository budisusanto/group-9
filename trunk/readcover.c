#include <stdio.h>
#include <stdlib.h>
#include "bmpr.h"

unsigned char matrix[8][8];
//reads cover image, returns the number of pixels left in the image
//select color to be used for hidding 'r' for red, 'g' for green, 'b' for blue
int readCover(FILE *pFile, int pixread, int width, char rgb){
   RGB color;
   int level, i;
   
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
   if((pixread % (width*8))!=0)
      fseek(pFile, (54 + (sizeof(RGB)*(pixread/8))), SEEK_SET);
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

       while(pixread<totalpix){
          pixread = readCover(pFile,pixread,bmpInfoHdr.biWidth,'b');
       }
       for(i=0;i<8;i++){
          for(j=0;j<8;j++){
             printf("%x ", matrix[i][j]);
	  }
          printf("\n");
       }
       printf("pixels left: %d\n",(totalpix-pixread));

        return 0;
}







