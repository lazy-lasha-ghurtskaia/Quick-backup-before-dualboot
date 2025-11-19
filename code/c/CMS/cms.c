#define _GNU_SOURCE
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
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

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
	int listenfd, optval=1;
	struct addrinfo *list, *p, hints;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_ADDRCONFIG;
	getaddrinfo(NULL, port, &hints, &list);

	for(p = list; p; p=p->ai_next){
		if((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) <= 0){
			continue;
		}
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
				(const void *)&optval , sizeof(int));
		if((bind(listenfd, p->ai_addr, p->ai_addrlen)) == 0)
			break;
		close(listenfd);
	}
	freeaddrinfo(list);
	if (!p)
		return -1;

	if (listen(listenfd, 1024) < 0){
		close(listenfd);
		return -1;
	}

	return listenfd;
}

int parse_uri(char* uri, char* filename, char* args){
	char *ptr;
	char *n;
	if(!strstr(uri, "cgi-bin")){
		strcpy(args, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		if (uri[strlen(uri)-1] == '/')
			strcat(filename, "index.html");
		return 1;
	}
	else {
		ptr = index(uri, '?');
		if (ptr) {
			strcpy(args, ptr+1);
			*ptr = '\0';
		} else 
			strcpy(args, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		return 0;
	}
}

void gettype(char *name, char* type){
	if (strcmp(name, ".html"))
		strcpy(type, "text/html");
	else if (strcmp(name, ".gif"))
		strcpy(type, "image/gif");
	else if (strcmp(name, ".png"))
		strcpy(type, "image/png");
	else if (strcmp(name, ".jpeg"))
		strcpy(type, "image/jpeg");
	else 
		strcpy(type, "text/plain");
}

void handle_static(int fd, char *filename, int size){
	int srcfd;
	char buf[1024], type[12];
	char *srcp;

	gettype(filename, type);

	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Small Concurrent Server\r\n", buf);
	sprintf(buf, "%sContent-length: %d\r\n", buf, size);
	sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, type);
	rio_writen(fd, buf, strlen(buf));
	printf("%s\n", buf);

	if ((srcfd = open(filename, O_RDONLY, 0)) <= 0){ 
		printf("Open error: %s\n", strerror(errno));
	}

	srcp = mmap(0, size, PROT_READ, MAP_PRIVATE, srcfd, 0);
	close(srcfd);
	rio_writen(fd, srcp, size);
	munmap(srcp, size);
}

void handle_dynamic(int fd, char* filename, char* args){
	// fork a child and have it run file with args
	// prep args
	int pid;
	char buf[1024], *emptylist[] = {NULL};

	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Small Concurrent Server\r\n", buf);
	sprintf(buf, "%sContent-type: text/html\r\n\r\n", buf);
	rio_writen(fd, buf, strlen(buf));

	if ((pid = fork()) == 0){
		// i'm the child
		setenv("QUERY_STRING", args, 1);
		dup2(fd, STDOUT_FILENO);
		execve(filename, emptylist, environ);
	}
	wait(NULL);
}

void skip_headers(rio_t *rio){
	char buf[1024];
	rio_readlineb(rio, buf, 1024);
	while(strcmp(buf, "\r\n")){
		rio_readlineb(rio, buf, 1024);
		printf("%s", buf);
	}
}

void handle(int fd){
	struct stat sb;
	rio_t rio;
	char buf[8192];
	int is_static;
	char req_type[1024], uri[1024], ver[1024];
	char filename[1024], args[1024];

	rio_readinitb(&rio, fd);
	rio_readlineb(&rio, buf, 8192);
	printf("Request headers:\n %s", buf);
	sscanf(buf, "%s %s %s", req_type, uri, ver);

	if (strcasecmp(req_type, "GET")){
		// write client error for this
		printf("Request not supported\n");
		return;
	}
	skip_headers(&rio);

	is_static = parse_uri(uri, filename, args);
	if (stat(filename, &sb)){
		// write client error for this
		printf("stat error: %s\n", strerror(errno));
		return;
	}

	if (is_static){
		if (( S_ISREG(sb.st_mode) ) && (sb.st_mode & S_IRUSR)){
			handle_static(fd, filename, sb.st_size);
		}
		else {
			// write client error for this
			printf("Not a file or no read permission\n");
		}
		return;
	} 
	else {
		if (( S_ISREG(sb.st_mode) ) && (sb.st_mode & S_IXUSR)){
			handle_dynamic(fd, filename, args);
		}
		else {
			// write client error for this
			printf("Not a file or no exec permission\n");
		}
		return;
	}

}

void* thread(void* targ){
	int clientfd;

	pthread_detach(pthread_self());

	while(1){
		clientfd = sbuf_remove(&fd_buf);
		printf("serving clientfd: %d\n", clientfd);
		handle(clientfd);	
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
		printf("* getting new ones *\n");
		len = sizeof(struct sockaddr_storage);
		clientfd = accept(listenfd, (struct sockaddr*)&addr, &len);
		sbuf_insert(&fd_buf, clientfd);
	}
}
