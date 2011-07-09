#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 8
#define PI 3.14159265

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
	
	return 0;
}
