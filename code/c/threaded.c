#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define N 10

void P(sem_t *sem){
	sem_wait(sem);
}
void V(sem_t *sem){
	sem_post(sem);
}

typedef struct{
	int *buf;
	int n;
	int front;
	int rear;
	sem_t mutex;
	sem_t slots;
	sem_t ready;
} sbuf_t;

void sbuf_init(sbuf_t *st, int n){
	st->buf = calloc(n, sizeof(int));
	st->n = n;
	st->front = 0;
	st->rear = 0;
	sem_init(&st->mutex, 0, 1);
	sem_init(&st->slots, 0, n);
	sem_init(&st->ready, 0, 0);
}

void sbuf_deinit(sbuf_t *st){
	free(st->buf);
}

void sbuf_insert(sbuf_t *st, int val){
	P(&st->slots);
	P(&st->mutex);
	st->buf[(++st->rear) % (st->n)] = val;
	V(&st->mutex);
	V(&st->ready);
}
int sbuf_remove(sbuf_t *st){
	int val;
	P(&st->ready);
	P(&st->mutex);
	val = st->buf[(++st->front) % (st->n)];
	V(&st->mutex);
	V(&st->slots);
	return val;
}

sbuf_t sbuf;
void* genval(void* smh){
	int i;
	pthread_detach(pthread_self());
	for (i = 0; i< N; i++){
		sleep(1);
		sbuf_insert(&sbuf, i);
	}
}

void* printval(void* smh){
	int i;
	pthread_detach(pthread_self());
	for (i = 0; i< N; i++){
		printf("%d ", sbuf_remove(&sbuf));
		fflush(stdout);
	}
	printf("\n");
}

int main(){
	// two threads: 
	// 		1 creates a bunch of values through some computation
	// 		2 retreives and prints these values
	pthread_t tid1, tid2;	

	sbuf_init(&sbuf, N);
	sbuf_deinit(&sbuf);
	pthread_create(&tid1, NULL, genval, NULL);
	pthread_create(&tid2, NULL, printval, NULL);
	pthread_exit(0);
}
