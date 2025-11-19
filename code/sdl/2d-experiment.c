#include <stdlib.h>
#include <stdio.h>
int main()
{
	int **nums = malloc(2*sizeof(int));
	nums[0] = (int*) malloc(2*sizeof(int));
	nums[1] = (int*) malloc(2*sizeof(int));
	for (int i = 0; i< 2; i++){
		for (int j = 0; j< 2; j++)
			nums[i][j] = i+j;
	}
		
	for (int i = 0; i< 2; i++){
		for (int j = 0; j< 2; j++)
			printf("%d", nums[i][j]);
		printf("\n");
	}
}
