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

void echo(int socket){
	char buf[8192];
	while(read(socket, buf, 1))
		write(socket, buf, 1);
}

void handle(){
	char buf[8192];
	if (!fgets(buf, 8192, stdin))
		exit(0);
	printf("%s", buf);
}

int main(int argc, char **argv){
	int listenfd, clientfd;
	socklen_t len;
	struct sockaddr_storage addr;
	fd_set fdset, readyset;
	int fdlen;

	if (argc != 2) usage;

	listenfd = open_listenfd(argv[1]);
	FD_ZERO(&fdset);
	FD_SET(0, &fdset);
	FD_SET(listenfd, &fdset);

	while(1){
		readyset = fdset;
		select(listenfd+1, &fdset, NULL, NULL, NULL);

		if (FD_ISSET(0, &readyset))
			handle();
		if (FD_ISSET(listenfd, &readyset)){
			len = sizeof(struct sockaddr_storage);
			clientfd = accept(listenfd, (struct sockaddr*)&addr, &len);
			echo(clientfd);
			close(clientfd);
		}
	}
	close(listenfd);
}
