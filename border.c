#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void build_zero_array(uint8_t array[][8], int arraySize)
{
	int x, y;
	for(x = 0; x < arraySize; x++)
		for(y = 0; y < arraySize; y++)
			array[x][y] = 0;
}

void build_WC_array(uint8_t array[][8], int arraySize)
{
	int x, y;
	for(x = 0; x < arraySize; x++)
	{
		for(y = 0; y < arraySize; y++)
		{
			
			if (x % 2 == 0)
			{
				if (y % 2 == 0)
					array[x][y] = 0;
				else
					array[x][y] = 1;
			}

			else
			{
				if (y % 2 == 0)
					array[x][y] = 1;
				else
					array[x][y] = 0;
			}

		}
	}
}

void build_BC_array(uint8_t array[][8], int arraySize)
{
	int x, y;
	for(x = 0; x < arraySize; x++)
	{
		for(y = 0; y < arraySize; y++)
		{
			if (x % 2 == 0)
			{
				if (y % 2 == 0)
					array[x][y] = 1;
				else
					array[x][y] = 0;
			}

			else
			{
				if (y % 2 == 0)
					array[x][y] = 0;
				else
					array[x][y] = 1;
			}
		}
	}
}

void print_8x8_array(uint8_t array[][8])
{
	int x, y;
	
	printf("\n---- smoke weed\n");
	for(x = 0; x < 8; x++)
	{
		for(y = 0; y < 8; y++)
		{
			printf("%u ", array[x][y]);
		}

		printf("\n");
	}
}

float calc_complex(uint8_t array[][8])
{
	int x, y, sum = 0;
	
	for (x = 0; x < 8; x++)
	{
		for (y = 0; y < 8; y++)
		{
			if (y != 7)
				if (array[x][y] != array[x][y+1])
					sum++;

			if (x != 7)
				if (array[x][y] != array[x+1][y])
					sum++;
		}
	}

	return sum / 112.0;
}

int main(int argc, char *argv[])
{
	uint8_t byteArray[8][8];
	int x, y, sum = 0;

	build_zero_array(byteArray, 8);

	byteArray[0][7] = 1;
	byteArray[7][7] = 1;
	byteArray[5][5] = 1;

	print_8x8_array(byteArray);
	printf("complexity: %f\n", calc_complex(byteArray));

	build_WC_array(byteArray, 8);
	print_8x8_array(byteArray);
	printf("complexity: %f\n", calc_complex(byteArray));
	
	build_BC_array(byteArray, 8);
	print_8x8_array(byteArray);
	printf("complexity: %f\n", calc_complex(byteArray));

	//printf("border length: %d\n", 1%2);
	return 0;
}
