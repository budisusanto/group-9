#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>

static int cgc[256],pbc[256];


int main(){

	int testArray[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,
			   14,15,16,17,18,19};
	int k,length = 20;
	
	for(k = 0;k < length;k++){
		printf("Converting: %d\n",k);
		printf("\t");
		cgcConverter(k);
		printf("\n");
	}
	  
}

void cgcConverter(int num){
	int i,k,j = 0, cnt = 0;
	
	while(num!=0)
	{
		pbc[j] = num % 2;
		num/=2;
		j++;
	}

	cgc[0] = pbc[7];
	
	for(i = 8;i > 0;i--){
		if(pbc[i] == 0 && pbc[i-1] == 0)
			cgc[cnt] = 0;
		else if(pbc[i] == 1 && pbc[i-1] == 1)
			cgc[cnt] = 0;
		else if(pbc[i] == 0 && pbc[i-1] == 1)
			cgc[cnt] = 1;
		else if(pbc[i] == 1 && pbc[i-1] == 0)
			cgc[cnt] == 1;
		cnt++;
	}
	
	for(k = 0;k < 8;k++)
		printf("%d",cgc[k]);

}

