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

#include "proxy.h"

#define PORT "8080"

void http_request_init(http_request *req)
{
	req = (http_request*)malloc(sizeof(http_request));
	req->method = -1; 
	req->search_path = NULL; 
}

void http_request_destroy(http_request *req)
{
}

void http_request_print(http_request *req)
{
	printf("HTTP_REQUEST: \n"); 
	printf("method: %s\n", 
		http_methods[req->method]);
	printf("search path: %s\n", 
		req->search_path); 
}


void http_parse_method(http_request *result, char *line)
{
	char *str_part = strtok(line, " ");
	
	int method, found = 0; 
	for(method = 0; method < http_methods_len; method++)
	{
		if(strcmp(str_part, http_methods[method]) == 0)
		{
			found = 1; 
			result->method = method;
			break; 
		}
	}

	if(found)
	{
		return; 
	}

	// Get the search path from the request
	// (perhaps this should only be done when we
	// get a GET request?)
	str_part = strtok(NULL, " "); 
	result->search_path = strdup(str_part); 
}

char *read_line(int sockfd)
{
	int buffer_size = 2; 
	char *line = (char*)malloc(sizeof(char)*buffer_size+1); 
	char c;
	int length = 0;	
	int counter = 0;

	while(1) 
	{
		length = recv(sockfd, &c, 1, 0); 
		line[counter++] = c; 

		if(c == '\n')
		{
			line[counter] = '\0'; 
			return line; 
		}

		// reallocate the buffer 
		if(counter == buffer_size)
		{
			buffer_size *= 2; 
			line = (char*)realloc(line, sizeof(char)*buffer_size); 
		}

	}

	return NULL;
}

void handle_client(int sockfd)
{
	char *line; 
	char *request = NULL;
	http_request *req; 
	http_request_init(req); 

	printf("reading line\n"); 
	line = read_line(sockfd); 
	printf("read line: %s\n", line); 

	printf("parsing method\n"); 
	http_parse_method(req, line); 
	printf("done parsing method\n"); 

	http_request_print(req); 
}

void start_server(unsigned int port)
{
	printf("Starting server\n"); 

	int sockfd, new_fd; 
	struct addrinfo hints, *servinfo, *p; 
	struct sockaddr_storage their_addr; 
	socklen_t sin_size; 
	struct sigaction sa; 
	int rv; 
	int yes = 1; 
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM; 
	hints.ai_flags = AI_PASSIVE; 

	if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return; 
	}

	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if((sockfd = socket(p->ai_family, p->ai_socktype, 
			p->ai_protocol)) == -1) 
		{
			perror("server: socket"); 
			continue; 
		}

		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, 
			sizeof(int)) == -1) 
		{
			perror("setsockopt"); 
			exit(1); 
		}

		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd); 
			perror("server: bind"); 
			continue; 
		}

		break; 
	}

	if(p == NULL)
	{
		fprintf(stderr, "server: failed to bind\n"); 
		return; 
	}

	freeaddrinfo(servinfo); 

	if(listen(sockfd, 10) == -1)
	{
		perror("listen"); 
		exit(1); 
	}

	printf("server: waiting for connections..\n"); 
	while(1) 
	{
		sin_size = sizeof(their_addr); 
		new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size); 
		if(new_fd == -1)
		{
			perror("accept"); 
			continue; 
		}
		
		printf("Receieved connection\n"); 

		pid_t child_pid = fork();
		if(!child_pid) 
		{
			printf("Forked\n"); 

			printf("handling client\n"); 
			handle_client(new_fd); 
			printf("done handling client\n"); 

			close(new_fd); 
			exit(0); 
		}
		close(new_fd); 
	}
}

int main(int argc, char *argv[])
{
	start_server(8080); 
	return 0; 
}
