#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/queue.h>

#include "proxy.h"

/**
Creates a TCP connection to the given host 
and returns the socket. Returns -1 if something
fails.
*/
int http_connect(const char *host, const char *port)
{
	struct addrinfo hints, *servinfo, *p; 
	int sockfd, rv; 

	memset(&hints, 0, sizeof hints); 
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM; 

	if((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0)
	{
		printf("Failed to lookup hostname\n");
		return -1; 
	}
	
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return -1;
	}

	return sockfd;
}

/**
Read a HTTP header from the given socket and
returns a http_request*. 
*/
http_request *http_read_header(int sockfd)
{
	return NULL;
}

/**
Read as much data as possible from the given socket
and returns it as a null terminated char pointer. Data 
returned from this function must be freed somewhere else. 
*/
char *http_read_chunk(int sockfd)
{
	return NULL; 
}
