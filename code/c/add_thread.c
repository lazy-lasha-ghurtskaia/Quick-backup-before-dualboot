#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
void usage(){
	printf("usage: <progname>  <threads> <2^? to add>\n");
	exit(0);
}

#define THREADS 32
int per_thread_work;
long long ncomputed[THREADS];

void* thread(void* vargp){
	long id = (long)vargp;
	long start = id * per_thread_work;
	long end = start + per_thread_work;
	long long locsum = 0;

	for (; start < end; start++)
		locsum += start;
	
	ncomputed[id] = locsum;
}

int main(int argc, char **argv){
	pthread_t tid[THREADS];
	long long nthreads, tot_nums, i;
	long long ntot = 0;

	if (argc != 3) usage();

	nthreads = atoi(argv[1]);
	tot_nums = (1L << atoi(argv[2]));
	per_thread_work = tot_nums/nthreads;

	for (i = 0; i< nthreads; i++){
		ncomputed[i] = 0;
		pthread_create(&tid[i], NULL, thread, (void*)i);
	}
	
	for (i = 0; i< nthreads; i++)
		pthread_join(tid[i], NULL);

	for (i = 0; i< nthreads; i++)
		ntot += ncomputed[i];

	long long expected = (tot_nums * (tot_nums-1))/2;
	printf("nthreads = %ld, tot_nums = %ld, per_thread_work = %d\n", 
       nthreads, tot_nums, per_thread_work);
	if (ntot !=  expected)
		printf("failure! ntot = %lld, expected = %lld\n", ntot);
	else 
		printf("success! ntot = %lld, expected = %lld\n", ntot);
}
