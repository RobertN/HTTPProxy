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
#include <time.h>

#include "proxy.h"
#include "list.h"
#include "http_message.h"

/*
Creates a TCP connection to the given host 
and returns the socket. Returns -1 if something
fails.
*/
int http_connect(http_request *req) 
{
	char *host = (char*)list_get_key(&req->metadata_head, "Host"); 
    char *port = strstr(host, ":");

    if(port == NULL)
    {
        // set port to default
        port = calloc(3, sizeof(char));
        strncat(port, "80", 2);

        LOG(LOG_TRACE, "Using default port\n");
    }
    else
    {
        // remove the port number from the host
        host = strtok(host, ":");

        // jump over the ':' char
        port++;

        LOG(LOG_TRACE, "Using port: %s\n", port);
    }
    

	LOG(LOG_TRACE, "Connecting to HTTP server: %s\n", host);

	if(host == NULL)
	{
		LOG(LOG_ERROR, "Could not find the Host property in the metadata\n");
		return -1; 
	}

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

/*
Read a HTTP header from the given socket and
returns a http_request*. 
*/
http_request *http_read_header(int sockfd)
{
	LOG(LOG_TRACE, "Reading header\n");
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
            LOG(LOG_TRACE, "Received header\n");
               
			break; 

		}

		http_parse_metadata(req, line); 

		free(line); 
	}

	return req;
}

/*
Read as much data as possible from the given socket
and returns it as a null terminated char pointer. Data 
returned from this function must be freed somewhere else. 
*/
char *http_read_chunk(int sockfd, ssize_t *length)
{
    if(length == NULL)
    {
        LOG(LOG_ERROR, "The length pointer supplied to http_read_chunk is NULL\n");
        return NULL;
    }

    if(sockfd == -1)
    {
        LOG(LOG_ERROR, "The socket given to http_read_chunk is invalid\n");
        return NULL;
    }

	char *buf = malloc(sizeof(char));
	memset(buf, '\0', sizeof(char));
	char c; 
	int current_size = 1; 

    time_t timeout = 5; 
    time_t start = time(NULL);

	ssize_t total_bytes = 0;
    ssize_t num_bytes = 0;

	while(1)
	{
        // check if we should timeout
        if(time(NULL) - start > timeout)
        {
            LOG(LOG_WARNING, "Request timed out\n");
            break; 
        }

		num_bytes = recv(sockfd, &c, 1, 0);

		if(num_bytes <= -1) 
		{
			break;
		}
		else if(num_bytes == 0)
		{
			break;
		}

        // reallocate the buffer so the new data will fit
		buf = realloc(buf, sizeof(char)*++current_size);
		buf[total_bytes] = c; 

		total_bytes += num_bytes; 
	}

	LOG(LOG_TRACE, "Received: %d\n", (int)total_bytes);

	*length = total_bytes; 

	return buf; 
}
