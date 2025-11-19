#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

// //////////////
// rio begins here
// //////////////

#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;                /* Descriptor for this internal buf */
    int rio_cnt;               /* Unread bytes in internal buf */
    char *rio_bufptr;          /* Next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* Internal buffer */
} rio_t;

void rio_readinitb(rio_t *rp, int fd) 
{
    rp->rio_fd = fd;  
    rp->rio_cnt = 0;  
    rp->rio_bufptr = rp->rio_buf;
}
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;

    while (rp->rio_cnt <= 0) {  /* Refill if buf is empty */
	rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, 
			   sizeof(rp->rio_buf));
	if (rp->rio_cnt < 0) {
	    if (errno != EINTR) /* Interrupted by sig handler return */
		return -1;
	}
	else if (rp->rio_cnt == 0)  /* EOF */
	    return 0;
	else 
	    rp->rio_bufptr = rp->rio_buf; /* Reset buffer ptr */
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;          
    if (rp->rio_cnt < n)   
	cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) 
{
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) { 
        if ((rc = rio_read(rp, &c, 1)) == 1) {
	    *bufp++ = c;
	    if (c == '\n') {
                n++;
     		break;
            }
	} else if (rc == 0) {
	    if (n == 1)
		return 0; /* EOF, no data read */
	    else
		break;    /* EOF, some data was read */
	} else
	    return -1;	  /* Error */
    }
    *bufp = 0;
    return n-1;
}
ssize_t rio_writen(int fd, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
	if ((nwritten = write(fd, bufp, nleft)) <= 0) {
	    if (errno == EINTR)  /* Interrupted by sig handler return */
		nwritten = 0;    /* and call write() again */
	    else
		return -1;       /* errno set by write() */
	}
	nleft -= nwritten;
	bufp += nwritten;
    }
    return n;
}

// //////////////
// rio ends here
// //////////////

#define CINIT 16

void usage(){
	printf("usage: <progname>  <port> <threads>\n");
	exit(1);
}

void P(sem_t *st) {sem_wait(st);}
void V(sem_t *st) {sem_post(st);}

typedef struct{
	int *buf;
	int n;
	int front;
	int rear;
	sem_t mutex;
	sem_t slots;
	sem_t ready;
} sbuf_t;

sbuf_t fd_buf;

void sbuf_init(sbuf_t *sb){
	sb->buf = calloc(CINIT, sizeof(int));
	sb->n = CINIT;
	sb->front = 0;
	sb->rear = 0;
	sem_init(&sb->mutex, 0, 1);
	sem_init(&sb->slots, 0, CINIT);
	sem_init(&sb->ready, 0, 0);
}
void sbuf_deinit(sbuf_t *sb){
	free(sb->buf);
}
void PS(sem_t *sb){
	int temp;
	sem_getvalue(sb, &temp);
	printf("%d\n", temp);
}
void sbuf_insert(sbuf_t *sb, int val){
	P(&sb->slots);
	P(&sb->mutex);
	sb->buf[(++sb->rear) % (sb->n)] = val;
	V(&sb->mutex);
	V(&sb->ready);
}
int sbuf_remove(sbuf_t *sb){
	int ret, temp;
	P(&sb->ready);
	P(&sb->mutex);
	ret = sb->buf[(++sb->front) % (sb->n)];
	V(&sb->mutex);
	V(&sb->slots);
	return ret;
}

int open_listenfd(char *port){
	//don't forget freeaddrinfo
	int listenfd;
	struct addrinfo *list, *p, hints;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
	getaddrinfo(NULL, port, &hints, &list);

	for(p = list; p; p=p->ai_next){
		if((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) <= 0){
			close(listenfd);
			continue;
		}
		if((bind(listenfd, p->ai_addr, p->ai_addrlen)) == 0)
			break;
	}
	listen(listenfd, 1024);

	return listenfd;
}

int bytes_rec;
sem_t echo_sem;

void echo_init(void){
	sem_init(&echo_sem, 0, 1);
	bytes_rec = 0;
}
void echo_cnt(int clientfd){
	// initialize echo_cnt shit: bytes-rec
	// initialize rio
	int n;
	char buf[RIO_BUFSIZE];
	rio_t rio;

	rio_readinitb(&rio, clientfd);

	while((n = rio_readlineb(&rio, buf, RIO_BUFSIZE)) > 0){
		P(&echo_sem);
		bytes_rec += n;
		printf("received %d (%d total) bytes on fd:%d", n, bytes_rec, clientfd);
		V(&echo_sem);
		rio_writen(clientfd, buf, n);
	}
}

void* thread(void* targ){
	int clientfd;
	pthread_once_t once = PTHREAD_ONCE_INIT;

	pthread_detach(pthread_self());
	pthread_once(&once, echo_init);

	while(1){
		clientfd = sbuf_remove(&fd_buf);
		echo_cnt(clientfd);	
		close(clientfd);
	}
}

int main(int argc, char **argv){
	pthread_t tid;
	int listenfd, clientfd, i;
	socklen_t len; 
	struct sockaddr_storage addr;

	if (argc != 3) usage();

	sbuf_init(&fd_buf);
	listenfd = open_listenfd(argv[1]);

	for(i = 0; i< atoi(argv[2]); i++)
		pthread_create(&tid, NULL, thread, NULL);

	while(1){
		len = sizeof(struct sockaddr_storage);
		clientfd = accept(listenfd, (struct sockaddr*)&addr, &len);
		sbuf_insert(&fd_buf, clientfd);
	}
}
