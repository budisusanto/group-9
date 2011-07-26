


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bmpr.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

//3D array to store bit slices
unsigned char bitslices[8][8][8];

// 8x8 checker board matrix
unsigned char checker[8][8] =
	{{0,1,0,1,0,1,0,1},
         {1,0,1,0,1,0,1,0},
         {0,1,0,1,0,1,0,1},
         {1,0,1,0,1,0,1,0},
         {0,1,0,1,0,1,0,1},
         {1,0,1,0,1,0,1,0},
         {0,1,0,1,0,1,0,1},
         {1,0,1,0,1,0,1,0}};

// Global Variables for File Data Pointers
BITMAPFILEHEADER *pSrcFileHdr, *pTgtFileHdr;
BITMAPINFOHEADER *pSrcInfoHdr, *pTgtInfoHdr;
RGBQUAD *pSrcColorTable, *pTgtColorTable;
unsigned char *pSrcFile, *pTgtFile, *pSrcData, *pTgtData;
int srcFileSize = 0, tgtFileSize;

unsigned char toCGC[256];
unsigned char toPBC[256];

// default values
unsigned char gNumLSB = 1, gMask = 0xfe, gShift = 7;

// sets bit indicator on a 8x8 matrix
// at the bottom left bit, 1 if exists conjugation
// 0 otherwise
void set_indicator_to(int x, unsigned char matrix[8][8]){
   if(x==1||x==0)
      matrix[0][7] = x;
}

// xor's an uncomplex matrix with a checker board matrix
// to make it complex and sets bit indicator to 1
void conjugate(unsigned char ncomp[8][8]){
   int i, j;
   for(i=0;i<8;i++){
      for(j=0;j<8;j++){
         ncomp[i][j] = checker[i][j] ^ ncomp[i][j];
      }
   }
   set_indicator_to(1,ncomp);
}



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
void getbitplanes(unsigned char matrix[8][8]){
   int x, y, z;
   for(x=0;x<8;x++){
      for(y=0;y<8;y++){
         for(z=0;z<8;z++){
            bitslices[x][y][z] = getBit(matrix[x][y],z);
         }
      }
   }
}


// this function builds the canonical gray code array variables
void buildGrayCode(){
        int i, length, posUp, posDw, cnt;

        length = 1;
        toCGC[0] = 0;
        toPBC[0] = 0;
        cnt = 1;

        while(length < 256)
        {
                posUp = length - 1;
                posDw = length;
                for(i = 0; i < length; i++)
                {
                        toCGC[i + posDw] = toCGC[posUp - i] + length;
                        toPBC[toCGC[i + posDw]] = cnt++;
                }
                length = length * 2;
        }
        return;
} // buildGrayCode

// Show the various bitmap header bytes primarily for debugging
void displayFileInfo(char *pFileName,
                                         BITMAPFILEHEADER *pFileHdr, 
                                         BITMAPINFOHEADER *pFileInfo,
                                         RGBQUAD *pColorTable,
                                         unsigned char *pData){
        // first two characters in bitmap file should be "BM"
        char *pp = (char *) &(pFileHdr->bfType);
        int numColors, i;
        RGBQUAD *pCT = pColorTable;

        printf("\nFile Info for %s: \n\n", pFileName);

        printf("File Header Info:\n");
        printf("File Type: %c%c\nFile Size:%d\nData Offset:%d\n\n", 
                *pp, *(pp + 1), pFileHdr->bfSize, pFileHdr->bfOffBits);

        switch(pFileInfo->biBitCount)
        {
        case 1:
                numColors = 2;
                break;
        case 4:
                numColors = 16;
                break;
        case 8:
                numColors = 256;
                break;
        case 16:
                numColors = 65536;
                break;
        case 24:
                numColors = 16777216;
                break;
        default:
                numColors = 16777216;
        }

        printf("Bit Map Image Info:\n\nSize:%d\nWidth:%d\nHeight:%d\nPlanes:%d\n"
                "Bits/Pixel:%d ==> %d colors\n"
                "Compression:%d\nImage Size:%d\nRes X:%d\nRes Y:%d\nColors:%d\nImportant Colors:%d\n\n",
                pFileInfo->biSize, 
                pFileInfo->biWidth, 
                pFileInfo->biHeight, 
                pFileInfo->biPlanes, 
                pFileInfo->biBitCount, numColors,
                pFileInfo->biCompression, 
                pFileInfo->biSizeImage, 
                pFileInfo->biXPelsPerMeter,
                pFileInfo->biYPelsPerMeter,
                pFileInfo->biClrUsed,
                pFileInfo->biClrImportant);

        //* Display Color Tables if Desired
        
        //      There are no color tables
        if(pFileInfo->biBitCount > 16) return;

        //      only needed for debugging
        printf("Color Table:\n\n");

        for(i = 0; i < numColors; i++)
        {
                printf("R:%02x   G:%02x   B:%02x\n", pCT->rgbRed, pCT->rgbGreen, pCT->rgbBlue);
                pCT++;
        }
        //*/

        return;
} // displayFileInfo


// writes modified bitmap file to disk
// gMask used to determine the name of the file
int writeFile(unsigned char *pFile, int fileSize, char *fileName){
        FILE *ptrFile;
        char newFileName[256], msk[4];
        int x;

        // convert the mask value to a string
        sprintf(msk, "%02x", gMask);

        // make a new filename based upon the original
        strcpy(newFileName, fileName);

        // remove the .bmp (assumed)
        x = (int) strlen(newFileName) - 4;
        newFileName[x] = 0;

        strcat(newFileName, "_mask_");
        strcat(newFileName, msk);       // name indicates which bit plane(s) was/were saved

        // add the .bmp back to the file name
        strcat(newFileName, ".bmp");

        // open the new file, MUST set binary format (text format will add line feed characters)
        ptrFile = fopen(newFileName, "wb+");
        if(ptrFile == NULL)
        {
                printf("Error opening new file for writing.\n\n");
                return(FAILURE);
        }

        // write the file
        x = (int) fwrite(pFile, sizeof(unsigned char), fileSize, ptrFile);

        // check for success
        if(x != fileSize)
        {
                printf("Error writing file %s.\n\n", newFileName);
                return(FAILURE);
        }
        fclose(ptrFile); // close file
        return(SUCCESS);
} // writeFile

// prints help message to the screen
void printHelp(){
        printf("Usage: Steg_LSB 'source filename' 'target filename' [bits to use] \n\n");
        printf("Where 'source filename' is the name of the bitmap file to hide.\n");
        printf("Where 'target filename' is the name of the bitmap file to conceal the source.\n");
        printf("To extract data from the source, name the target file \"ex\".\n");
        printf("To bit slice the source, name the target file \"bs\".\n");
        printf("The number of bits to hide or extract, range is (1 - 7).\n");
        printf("If not specified, 1 bit is used as the default.\n\n");
        return;
} // printHelp

// scramble the color table
unsigned char *scramble(unsigned char *pSrcFile, RGBQUAD *pSrcColorTable, int fileSize){
        BITMAPINFOHEADER *pDstInfoHdr;
        RGBQUAD *pDstColorTable;
        int i, cnt = 0, last = 0;
        unsigned char *pDstFile;

        pDstFile = (unsigned char *) malloc(fileSize);

        if(pDstFile == NULL)
        {
                printf("\n\nError allocating memory for scrambled file.\n\n");
                return(NULL);
        }

        // copy the source file to memory
        memcpy(pDstFile, pSrcFile, fileSize);

        pDstInfoHdr = (BITMAPINFOHEADER *) (pDstFile + sizeof(BITMAPFILEHEADER));
        // pointer to first RGB color palette, follows file header and bitmap header
        pDstColorTable = (RGBQUAD *) (pDstFile + sizeof(BITMAPFILEHEADER) + pDstInfoHdr->biSize);

        // the color table has 256 entries
        // will scramble by choosing the next color to swap, using a prime number
        // by using a number relatively prime to 256, we will hit each and every color
        // table entry exactly once
        for(i = 0; i < 256; i++)
        {
                cnt += 67;      // using 211 as our number relatively prime to 256
                if(cnt > 255) cnt -= 256;
printf("color %d == color %d\n", last,cnt);
                pDstColorTable[last].rgbBlue = pSrcColorTable[cnt].rgbBlue;
                pDstColorTable[last].rgbGreen = pSrcColorTable[cnt].rgbGreen;
                pDstColorTable[last].rgbRed = pSrcColorTable[cnt].rgbRed;

                last += 1;
                if(last > 255) last -= 256;
        }
        return(pDstFile);
} // scramble

/*
int pow(int x, int y){
   if(y==0)
      return 1;

   int i, result;
   result = x;

   for(i=1;i<y;i++){
      result *= x;
   }

   return result;
}*/

// reads specified bitmap file from disk
unsigned char *readFile(char *fileName, int *fileSize){
        FILE *ptrFile;
        unsigned char *pFile;
	int fsize;
        struct stat filebuf;

        ptrFile = fopen(fileName, "rb");        // specify read only and binary (no CR/LF added)

        if(ptrFile == NULL)
        {
                printf("Error in opening file: %s.\n\n", fileName);
                return(NULL);
        }

        
        // get file size
        stat(fileName, &filebuf);
        
	fsize = filebuf.st_size;
        *fileSize = fsize;

	printf("file size variable: %d\n",fsize);
	printf("file size pointer: %d\n",*fileSize);
	        
        //*fileSize = filelength(fileno(ptrFile));

        // malloc memory to hold the file, include room for the header and color table
        pFile = (unsigned char *) malloc(*fileSize);

        if(pFile == NULL)
        {
                printf("Memory could not be allocated in readFile.\n\n");
                return(NULL);
        }

        // Read in complete file
        // buffer for data, size of each item, max # items, ptr to the file
        fread(pFile, sizeof(unsigned char), *fileSize, ptrFile);
        fclose(ptrFile);
        return(pFile);
} // readFile


// Main function in LSB Steg
// Parameters are used to indicate the input file and available options
int main(int argc, char *argv[]){
        int x;
	char *pp;
	int *srcsize;	

        if(argc < 3 || argc > 4)
        {
                printHelp();
                return;
        }

        // initialize gray code conversion table
        buildGrayCode();

        // get the number of bits to use for data hiding or data extracting
        // if not specified, default to one
        if(argc == 4)
        {
                // the range for gNumLSB is 1 - 7;  if gNumLSB == 0, then the mask would be 0xFF and the
                // shift value would be 8, leaving the target unmodified during embedding or extracting
                // if gNumLSB == 8, then the source would completely replace the target
                gNumLSB = *(argv[3]) - 48;
                if(gNumLSB < 1 || gNumLSB > 7)
                {
                        gNumLSB = 1;
                        printf("The number specified for LSB was invalid, using the default value of '1'.\n\n");
                }
                gMask = 256 - (int) pow(2.0, gNumLSB);
                gShift = 8 - gNumLSB;
        }

        // read the source file
        pSrcFile = readFile(argv[1], &srcFileSize);
        if(pSrcFile == NULL) return;

        // Set up pointers to various parts of the source file
        pSrcFileHdr = (BITMAPFILEHEADER *) pSrcFile;
        pSrcInfoHdr = (BITMAPINFOHEADER *) (pSrcFile + sizeof(BITMAPFILEHEADER));
        // pointer to first RGB color palette, follows file header and bitmap header
        pSrcColorTable = (RGBQUAD *) (pSrcFile + sizeof(BITMAPFILEHEADER) + pSrcInfoHdr->biSize);
        // file header indicates where image data begins
        pSrcData = pSrcFile + pSrcFileHdr->bfOffBits;

	pp = (char *) &(pSrcFileHdr->bfType);
	printf("File Type: %c%c\n", *pp, *(pp+1) );
	
	srcsize = (int *) &(pSrcFileHdr->bfSize);
	printf("File Size: %d\n", *srcsize);




/*
        // for debugging purposes, show file info on the screen
        displayFileInfo(argv[1], pSrcFileHdr, pSrcInfoHdr, pSrcColorTable, pSrcData);

        // scramble the color table
        if(strcmp(argv[2], "sc") == 0)
        {
                char nm[128];
                unsigned char *pFile;
                FILE *ptrFile;

                strcpy(nm, "sc_");
                strcat(nm, argv[1]);

                // scramble the entire color table
                pFile = scramble(pSrcFile, pSrcColorTable, srcFileSize);
                if(pFile == NULL) return;

                // open the new file, MUST set binary format (text format will add line feed characters)
                ptrFile = fopen(nm, "wb+");
                if(ptrFile == NULL)
                {
                        printf("Error opening new file for writing.\n\n");
                        return;
                }

                // write the file
                x = (int) fwrite(pFile, sizeof(unsigned char), srcFileSize, ptrFile);

                // check for success
                if(x != srcFileSize)
                {
                        printf("Error writing file %s.\n\n", nm);
                        return;
                }
                fclose(ptrFile); // close file

                return;
        }

        // read the target file
        pTgtFile = readFile(argv[2], &tgtFileSize);
        if(pTgtFile == NULL) return;

        // Set up pointers to various parts of file
        pTgtFileHdr = (BITMAPFILEHEADER *) pTgtFile;
        pTgtInfoHdr = (BITMAPINFOHEADER *) (pTgtFile + sizeof(BITMAPFILEHEADER));
        pTgtColorTable = (RGBQUAD *) (pTgtFile + sizeof(BITMAPFILEHEADER) + pTgtInfoHdr->biSize);
        pTgtData = pTgtFile + pTgtFileHdr->bfOffBits;

        // for debugging purposes, show file info on the screen
        displayFileInfo(argv[2], pTgtFileHdr, pTgtInfoHdr, pTgtColorTable, pTgtData);

        // write the file to disk
        x = writeFile(pTgtFile, pTgtFileHdr->bfSize, argv[2]);


*/

        return 0;
} // main
