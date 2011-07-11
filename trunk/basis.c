#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 8
#define PI 3.14159265

double cof(int x);
void dct(unsigned char a[8][8]);
void dctInverse(float a[8][8]);

int main(int argc, char *argv[])
{
	int u, v, x, y, i;
	float basis[8][8][8][8];
	float xVal, yVal;

	for(u = 0; u < N; u++)
		for(v = 0; v < N; v++)
			for(x = 0; x < N; x++)
				for(y = 0; y < N; y++)
				{
					xVal = PI * u * ( 2 * x + 1 );
					xVal /= 2 * N;
					xVal = cos( xVal );
					
					yVal = PI * v * ( 2 * y + 1 );
					yVal /= 2 * N;
					yVal = cos( yVal );
					
					basis[u][v][x][y] = xVal * yVal;
				}
	
	for(x = 0; x < N; x++)
	{
		for(y = 0; y < N; y++)
		{
			printf("%f ",basis[0][1][x][y]);
		}
		printf("\n");
	}
	
	/*  Test for DCT calculation  */
	unsigned char input[8][8];
	float input2[8][8];
	for(u=0;u<8;u++){
	   for(v=0; v<8; v++){
	      input[u][v] = 0;		//matrix with all zero values
	      input2[u][v] = 0;
	   }
	}
	dct(input);
	dctInverse(input2);
	for(u=0;u<8;u++){
	   for(v=0; v<8; v++){
	      input[u][v] = 255;	//matrix with all values = 255
	      input2[u][v] = 255;
	   }
	}
	dct(input);
	dctInverse(input2);
	int random;
        for(u=0;u<8;u++){
	   for(v=0; v<8; v++){
	      random = rand()%256;
	      input[u][v] = random;	//matrix with random values 0 to 255
	      input2[u][v] = random;
	      srand(random+7);
	   }
	}
	dct(input);
	dctInverse(input2);
	return 0;
}

//Part 2
void dct(unsigned char a[8][8]){
   double b[8][8];
   double sum, incos1, incos2, n;
   sum = 0;
   n = 8;
   int x, y, u, v;

   printf("\n\tDCT Calculation\n\nInput matrix:\n");
   for(u=0; u<8;u++){
      for(v=0; v<8;v++){
         printf("%3d ", a[u][v]);
      }
      printf("\n");
   }

   for(u=0; u<8;u++){
      for(v=0; v<8;v++){
         for(x=0; x<8;x++){
            for(y=0; y<8;y++){
               incos1 = (PI*u*((2*x)+1))/(2*n);
               incos2 = (PI*v*((2*y)+1))/(2*n);
               sum += (a[x][y]-128)*cos(incos1)*cos(incos2);
            }
         }
         b[u][v] = (2/n)*cof(u)*cof(v)*sum;

      }
   }
   printf("\nResulting matrix:\n");
   for(u=0; u<8;u++){
      for(v=0; v<8;v++){
         printf("%f ", b[u][v]);
      }
      printf("\n");
   }
   printf("\n");

}
//Part 3

void dctInverse(float a[8][8]){
  double b[8][8];
   double sum, incos1, incos2, n;
   sum = 0;
   n = 8;
   int x, y, u, v;

   for(u=0; u<N;u++){
      for(v=0; v<N;v++){
         for(x=0; x<N;x++){
            for(y=0; y<N;y++){
               incos1 = (PI*u*((2*x)+1))/(2*n);
               incos2 = (PI*v*((2*y)+1))/(2*n);
               sum += (cos(incos1)*cos(incos2));

            }
         }
	 b[u][v] = ((2/n)*cof(u)*cof(v)*sum) / (a[x][y]-128);

      }
   }
   printf("\nResulting matrix (Inverse):\n");
   for(u=0; u<n;u++){
      for(v=0; v<n;v++){
         printf("%f ", b[u][v]);
      }
      printf("\n");
   }
   printf("\n");

}


/* Function to calculate C(u) and C(v)*/
double cof(int x){
   if(x==0)
      return (1/sqrt(2));
   else
      return 1.0;
}

