#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#define MAXBUF 1024
#define RIO_BUFSIZE 8192

void usage(){
	printf("<progname>: <port>");
	exit(0);
}

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

void rio_readinitb(rio_t *rp, int fd) 
{
    rp->rio_fd = fd;  
    rp->rio_cnt = 0;  
    rp->rio_bufptr = rp->rio_buf;
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

int open_listenfd(char *port)
{
	struct addrinfo *list, *p, hints;
	int listenfd;
	hints.ai_socktype = SOCK_STREAM;
	memset(&hints, 0, sizeof hints);
	hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE | AI_ADDRCONFIG;
	
	getaddrinfo(NULL, port, &hints, &list);
	for (p = list; p; p = p->ai_next){
		// socket > bind > listen > accept
		if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
			continue;
		}
		
		if ( bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
			break;
	}
	freeaddrinfo(list);
	if (listen(listenfd, 1024) != 0){
		printf("\"listen\" error");
		return -1;
	}
	return listenfd;
}
void echo(int clientfd){
	rio_t rio;
	int n;
	char buf[MAXBUF];
	rio_readinitb(&rio, clientfd);
	while((n = rio_readlineb(&rio, buf, MAXBUF)) != 0){
		printf("Accepted %d bytes\n", n);
		rio_writen(clientfd, buf, n);
	}
}

int Accept(int fd, struct sockaddr* addr, int *len){
	int nfd;
	nfd = accept(fd, addr, len);
	if (nfd == -1)
		printf("%s", strerror(errno));
	return nfd;
}


int main(int argc, char *argv[])
{
	int listenfd, clientfd;
	socklen_t len;
	struct sockaddr_storage addr;

	listenfd = open_listenfd(argv[1]);
	while (1) {
		len = sizeof(struct sockaddr_storage);
		if ((clientfd = Accept(listenfd, (struct sockaddr*)&addr, &len)) < 0)
			continue;
		echo(clientfd);
		close(clientfd);
	}
	close(listenfd);
}
