#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bmpr.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

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
int main(int argc, char *argv[]) {
	char *fileName = argv[1];
	struct stat filebuf;
	int fsize, result;
	char *dataArray;
	FILE * pFile;
	BITMAPFILEHEADER bmpFileHdr;
	BITMAPINFOHEADER bmpInfoHdr;
	char *string;
	char *point;
	unsigned char *bitmapImage;
	
	stat(fileName, &filebuf);
	fsize = filebuf.st_size;
	
	printf("file name: %s\n",fileName);
	printf("file size: %d\n",fsize);
	
	dataArray = (char *) malloc(fsize);
	if (dataArray == NULL) { fprintf(stderr,"malloc error\n"); exit(1); }
	
	//http://www.vbforums.com/showthread.php?t=261522
	//http://www.brackeen.com/vga/source/bc31/bitmap.c.html
	
	pFile = fopen(fileName,"rb");
	if (pFile == NULL) { fprintf(stderr,"fopen error\n"); exit(1); }
	
//	result = fread(dataArray, 1, fsize, pFile);
	//if (result != fsize) { fprintf(stderr,"fread error\n"); exit(1); }
	
	fread(&bmpFileHdr, 1, sizeof(struct tagBITMAPFILEHEADER), pFile);
	if (bmpFileHdr.bfType != 0x4D42) {
//		fclose(filePtr);
		fprintf(stderr,"Not BM\n");
//		return NULL;
	}

	//read the bitmap info header
	fread(&bmpInfoHdr, 1, sizeof(BITMAPINFOHEADER), pFile);

	//move file point to the begging of bitmap data
	fseek(pFile, bmpFileHdr.bfOffBits, SEEK_SET);

	//allocate enough memory for the bitmap image data
	bitmapImage = (unsigned char*)malloc(bmpInfoHdr.biSizeImage);


	string = (char *) &(bmpFileHdr.bfType);
	printf("File Type: %c%c\n", *string, *(string+1));
	printf("File Size: %d\n", bmpFileHdr.bfSize);
	printf("offBits: %d\n", bmpFileHdr.bfOffBits);

	printf("size of header: %d\nwidth: %d\nheight: %d\nplanes: %d\n\n",
		       	bmpInfoHdr.biSize,bmpInfoHdr.biWidth,bmpInfoHdr.biHeight,bmpInfoHdr.biPlanes);

/*	bmpFileHdr = (BITMAPFILEHEADER *)dataArray;
	string = (char *) &(bmpFileHdr->bfType);
	
	point = &(bmpFileHdr->bfType);
	
	printf("bfType addr: %p\n", point);
	point = (char *) &(bmpFileHdr->bfSize);
	printf("bfSize addr: %c%c\n", *point, *(point+1));
	printf("Hex @ bfSize[0]: %x\n", point);
	printf("Hex @ bfSize[1]: %x\n", ++point);
	printf("Hex @ bfSize[2]: %x\n", ++point);
	printf("Hex @ bfSize[3]: %x\n", ++point);
*/
        return 0;
}







