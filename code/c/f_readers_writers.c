#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

volatile int val;
sem_t w; //general access to val;
sem_t r; //access to numread;
volatile int numread;
int i = 0;

void P(sem_t *st) {sem_wait(st);}

void V(sem_t *st) {sem_post(st);}

void sbuf_write(){
	P(&w);
	val = ++i;
	//printf("++i\n");
	V(&w);
}
int sbuf_read(){
	int ret;
	P(&r);
	numread++;
	if (numread == 1)
		P(&w);
	V(&r);

	ret = val;
	//printf("ret val\n");

	P(&r);
	numread--;
	if (numread == 0)
		V(&w);
	V(&r);
	return ret;
}

void* reader(void* sval){
	while(1){
		sleep(1);
		printf("%d ", sbuf_read());
		fflush(stdout);
	}
}

void* writer(void* sval){
	while(1){
		sleep(1);
		sbuf_write();
	}
}


int main(){
	pthread_t tid1, tid2;

	sem_init(&w, 0, 1);
	sem_init(&r, 0, 1);

	pthread_create(&tid2, NULL, writer, NULL);
	pthread_create(&tid1, NULL, reader, NULL);
	pthread_exit(0);
}
