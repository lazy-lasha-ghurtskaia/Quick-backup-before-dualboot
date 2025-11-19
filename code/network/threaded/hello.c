#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

void P(sem_t *s){
	sem_wait(s);
}
void V(sem_t *s){
	sem_post(s);
}

volatile long numiter = 0;
sem_t sem;

void *func(void *niter){
	long i, loops = *(long*)niter;
	for (i = 0; i< loops; i++){
		P(&sem);
		numiter++;
		V(&sem);
	}
	return NULL;
}

int main(int argc, char **argv){
	pthread_t tid2, tid1;
	long niter = atoi(argv[1]);

	if (argc != 2) exit(0);

	sem_init(&sem, 0, 1);
	
	pthread_create(&tid1, NULL, func, &niter);
	pthread_create(&tid2, NULL, func, &niter);
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);

	if (numiter != (2*niter))
		printf("Boom!: %d\n", numiter);
	else 
		printf("Good!: %d\n", numiter);
}
