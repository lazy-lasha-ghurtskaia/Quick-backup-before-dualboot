#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int open_listenfd(char *port)
{
	struct addrinfo *p, *list, hints;
	int rs, listenfd, optval=1;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_ADDRCONFIG;
	getaddrinfo(NULL, port, &hints, &list);

	for (p = list; p; p = p->ai_next){
		// socket > bind > listen > accept
		if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
			continue;

		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
				(const void *)&optval, sizeof(int));

		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
			break;
		close(listenfd);
	}
	freeaddrinfo(list);
	if(!p)
		return -1;

	if (listen(listenfd, 1024) < 0){
		close(listenfd);
		return -1;
	}
	return listenfd;
}


int main(){
	int listenfd = open_listenfd("8080");
	printf("listenfd: %s\n", listenfd);
}
