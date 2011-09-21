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
