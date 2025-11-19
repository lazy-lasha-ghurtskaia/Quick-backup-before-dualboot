#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/select.h>

void usage(){
	printf("forgot your argument\n"); 
	exit(1);
}

int open_listenfd(char *port){
	struct addrinfo *list, *p, hints;
	int sock, ec;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
	if ((ec = getaddrinfo(NULL, port, &hints, &list)) != 0){
		printf("%s\n", gai_strerror(ec));
		return -1;
	}

	for (p = list; p; p=p->ai_next){
		if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 ){
			printf("%s\n", strerror(errno));
			continue;
		}

		if (bind(sock, p->ai_addr, p->ai_addrlen) == 0)
			break;
	}
	freeaddrinfo(list);
	if (sock < 0)
		return -1;
	if (listen(sock, 1024) != 0)
		return -1;
	return sock;
}
void client_overload(){
	printf("Too many clients");
	exit(0);
}

#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;                /* Descriptor for this internal buf */
    int rio_cnt;               /* Unread bytes in internal buf */
    char *rio_bufptr;          /* Next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* Internal buffer */
} rio_t;

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

void rio_readinitb(rio_t *rp, int fd) 
{
    rp->rio_fd = fd;  
    rp->rio_cnt = 0;  
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t rio_readn(int fd, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0) {
	if ((nread = read(fd, bufp, nleft)) < 0) {
	    if (errno == EINTR) /* Interrupted by sig handler return */
		nread = 0;      /* and call read() again */
	    else
		return -1;      /* errno set by read() */ 
	} 
	else if (nread == 0)
	    break;              /* EOF */
	nleft -= nread;
	bufp += nread;
    }
    return (n - nleft);         /* Return >= 0 */
}

typedef struct{
	fd_set read_set;
	fd_set ready_set;
	int nready;
	int maxfd;
	int maxi;
	int clientfd[256];
	rio_t riofd[256];
} pool_t;

void init_pool(int listenfd, pool_t* p){
	int i;
	p->maxi = -1;
	for (i=0; i<256; i++){
		p->clientfd[i] = -1;
	}

	FD_ZERO(&p->read_set);
	FD_SET(listenfd, &p->read_set);
	p->maxfd = listenfd;
}

void add_client(int clientfd, pool_t* p){
	int i;
	// from the book
	p->nready--;//
	for (i=0; i<256; i++)
		if (p->clientfd[i] < 0){
			p->clientfd[i] = clientfd;
			rio_readinitb(&p->riofd[i], clientfd);

			FD_SET(clientfd, &p->read_set);

			if (clientfd > p->maxfd)
				p->maxfd = clientfd;
			if (i > p->maxi)
				p->maxi = i;
			break;
		}
	if (i == 256)
		client_overload();
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

int total_bytes = 0;
void check_clients(pool_t* p){
	int clientfd;
	rio_t rio;
	int i;
	char buf[8192], n;

	for (i=0; ((i<=(p->maxi)) && ((p->nready)>0)); i++){
		clientfd = p->clientfd[i];
		rio = p->riofd[i];

		if ((clientfd > 0) && (FD_ISSET(clientfd, &p->ready_set))){//
			p->nready--;//
			if ((n = rio_readlineb(&p->riofd[i], buf, 8192)) != 0){
				total_bytes+=n;
				printf("We've got %d (%d total) bytes from fd:%d\n", n, total_bytes, clientfd);
				rio_writen(clientfd, buf, n);
			}
			else {
				close(clientfd);
				FD_CLR(clientfd, &p->read_set);
				p->clientfd[i] = -1;
			}
		}//
	}
}

int main(int argc, char **argv){
	int listenfd, clientfd;
	socklen_t len;
	struct sockaddr_storage addr;
	pool_t p;

	if (argc != 2) usage;

	listenfd = open_listenfd(argv[1]);
	init_pool(listenfd, &p);

	while(1){
		p.ready_set = p.read_set;
		p.nready = select(p.maxfd+1, &p.ready_set, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &p.ready_set)){
			len = sizeof(struct sockaddr_storage);
			clientfd = accept(listenfd, (struct sockaddr*)&addr, &len);
			add_client(clientfd, &p);
		}

		check_clients(&p);
	}
}
