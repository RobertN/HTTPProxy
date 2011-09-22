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
#include <unistd.h>
#include <fcntl.h>

#include "proxy.h"

/**
Creates a TCP connection to the given host 
and returns the socket. Returns -1 if something
fails.
*/
int http_connect(const char *host, const char *port)
{
	LOG(LOG_TRACE, "Connecting to HTTP server\n");

	struct addrinfo hints, *servinfo, *p; 
	int sockfd, rv; 

	memset(&hints, 0, sizeof hints); 
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM; 

	if((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0)
	{
		LOG(LOG_ERROR, "Failed to lookup hostname\n");
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
		LOG(LOG_ERROR, "Failed to connect to HTTP server\n");
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
/*
	http_request *req;
	http_request_init(&req); 

	char *line; 
	line = read_line(sockfd); 
	http_parse_method(req, line); 

	while(1) 
	{
		line = read_line(sockfd); 
		if(line[0] == '\r' && line[1] == '\n')
		{
			// We received the end of the HTTP header 
			break; 

		}
	}

*/
	return NULL;
}

/**
Read as much data as possible from the given socket
and returns it as a null terminated char pointer. Data 
returned from this function must be freed somewhere else. 
*/
char *http_read_chunk(int sockfd)
{
	char *buf = malloc(sizeof(char));
	memset(buf, '\0', sizeof(char));
	char c; 
	int current_size = 1; 

	// set the socket as non blocking
	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	while(1)
	{
		ssize_t num_bytes = recv(sockfd, &c, 1, 0);
		if(num_bytes == EAGAIN)
		{
			// read more
			continue; 
		}
		else if(num_bytes == -1) 
		{
			break;
		}

		buf = realloc(buf, sizeof(char)*++current_size);
		strncat(buf, &c, 1);
	}

	return buf; 
}
