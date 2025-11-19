#include <pthread.h>
#include <stdio.h>
#define N 2

char **ptr;
void *func(void* i);

int main(){
	// create two children
	// echo their id and string
	int i;
	pthread_t tid;
	char *share_strs[N] = {
		"Hello from first thread", 
		"Hello from second thread",
	};
	ptr = share_strs;

	for (i= 0; i< N; i++)
		pthread_create(&tid, NULL, func, (void*)&i);
	pthread_exit(NULL);
}
void *func(void* i){
	int fid = *(int*)i;
	static int cnt = 0;
	printf("[fid:%d] %s\n", fid, ptr[fid]);
	return NULL;
}
