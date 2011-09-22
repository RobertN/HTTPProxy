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
#include "net.h"
#include "list.h"

#define PORT "8080"



void http_request_init(http_request **req)
{
	*req = (http_request*)malloc(sizeof(http_request));

	http_request *request = *req; 
	request->method = 0; 
	request->search_path = NULL; 

	TAILQ_INIT(&request->metadata_head); 
}

void http_request_destroy(http_request *req)
{
}

void http_request_print(http_request *req)
{
	printf("[HTTP_REQUEST] \n"); 
	printf("method:\t\t%s\n", 
		http_methods[req->method]);
	printf("path:\t\t%s\n", 
		req->search_path); 

	printf("[Metadata] \n"); 
	struct http_metadata_item *item; 
	TAILQ_FOREACH(item, &req->metadata_head, entries) {
		printf("%s: %s\n", item->key, item->value); 
	}

	printf("\n"); 
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

	if(!found)
	{
		return; 
	}

	// Get the search path from the request
	// (perhaps this should only be done when we
	// get a GET request?)
	str_part = strtok(NULL, " "); 
	result->search_path = strdup(str_part); 

	// TODO: Retrieve the HTTP version
}

// Content-Byte: 101
void http_parse_metadata(http_request *result, char *line)
{
	char *key = strdup(strtok(line, ":")); 

	char *value = strdup(strtok(NULL, "\r")); 
	
	// remove whitespaces :)
	char *p = value; 
	while(*p == ' ') p++; 
	value = p; 

	http_metadata_item *item = (http_metadata_item*)malloc(sizeof(http_metadata_item)); 
	item->key = key; 
	item->value = value; 

	TAILQ_INSERT_TAIL(&result->metadata_head, item, entries); 
}

// TODO: Test all the edge cases
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

			// TODO: should probably allocate +1 for the null terminator,
			// but not sure.
			line = (char*)realloc(line, sizeof(char)*buffer_size); 
		}

	}

	return NULL;
}

int containing_forbidden_words(char str[]){

    // Forbidden words
    char *words[] = {"SpongeBob", "Britney Spears", "Paris Hilton", "Norrkӧping", "Norrk&ouml;ping", "Norrk%C3%B6ping"};
    int hits[] = {0, 0, 0, 0, 0, 0}; // Every forbidden word need to have a zero in this array to be able to count number of char hits.
    int numb_words = 6; // Number of forbidden words

    int str_length = strlen(str);
    int c, w;   // Index for char in str, and index for word in words

    // Search for forbidden words
    for (c = 0; c < str_length; c++)
    {
        for (w = 0; w < numb_words; w++)
        {
            if (words[w][ hits[w] ] == str[c]){
                if(++hits[w] == strlen(words[w]))
                    return 1;
            }
            else
                hits[w] = 0;
        }
    }

    return 0;
}

int send_to_client(int client_sockfd, char data[], int packages_size)
{
    // if packages_size is set to 0, then the function will try to send all data as one package.
    int length = strlen(data);
    if(packages_size < 1){
        if(send(client_sockfd, data, length, 0) == -1)
	    {
		    perror("Couldn't send data to the client.");
		    return -1;
	    }
    }
    else
    {
        int p;
        for(p = 0; p*packages_size + packages_size < length; p++){
            if(send(client_sockfd, (data + p*packages_size), packages_size, 0) == -1)
	        {
		        perror("Couldn't send any or just some data to the client. (loop)\n");
		        return -1;
            }
        }

        if (p*packages_size < length)
        {
            if(send(client_sockfd, (data + p*packages_size), length - p*packages_size, 0) == -1)
	        {
		        perror("Couldn't send any or just some data to the client.\n");
		        return -1;
            }
        }
    }
	printf("The data is sent to the client\n");
    return 0;
}

int http_request_send(int sockfd, http_request *req)
{
	const char *search_path = req->search_path; 
	//char request_buffer[] = "GET /index.html HTTP/1.1\r\nHost: 85.8.2.230\r\n\r\n"; 

	// construct the http request 
	int size = strlen("GET ") + 1; 
	char *request_buffer = malloc(sizeof(char)*size);
	memset(request_buffer, '\0', sizeof(char)*size);
	strncat(request_buffer, "GET ", 4);

	size += strlen(search_path) + 1; 
	request_buffer = realloc(request_buffer, size);
	strncat(request_buffer, search_path, strlen(search_path));

	size += strlen(" HTTP/1.0\r\n\r\n");
	request_buffer = realloc(request_buffer, size); 
	strncat(request_buffer, " HTTP/1.0\r\n\r\n", strlen(" HTTP/1.0\r\n\r\n"));

	// send the http request to the web server 
	if(send(sockfd, request_buffer, strlen(request_buffer), 0) == -1)
	{
		perror("send"); 
		return 1; 
	}

	LOG(LOG_TRACE, "Sent HTTP header to web server\n");
	
	return 0;
}

void handle_client(int client_sockfd)
{
	char *line; 
	int server_sockfd; 
	http_request *req; 
	http_request_init(&req); 

	line = read_line(client_sockfd); 
	http_parse_method(req, line); 

	while(1)
	{
		line = read_line(client_sockfd); 
		if(line[0] == '\r' && line[1] == '\n') 
		{
			// We received the end of the HTTP header 
			break; 
		}

		// TODO: Save the headers sent by the client in
		// a linked list or something.
		http_parse_metadata(req, line); 

		free(line); 
	}


	
	server_sockfd = http_connect(req);
	if(server_sockfd == -1) 
	{
		printf("Failed to connect to host\n");
		return; 
	}

	//printf("connected to host\n");
	LOG(LOG_TRACE, "Connected to host\n");

	// TODO: Send the request to the server 
	http_request_send(server_sockfd, req); 
	http_request_print(req);

	while(1)
	{
		line = read_line(server_sockfd); 
		send_to_client(client_sockfd, line, 0); 
		if(line[0] == '\r' && line[1] == '\n') 
		{
			// We received the end of the HTTP header 
			break; 
		}

		free(line); 
	}

	char *temp = http_read_chunk(server_sockfd);
	printf("\n\n%s\n", temp);
	send_to_client(client_sockfd, temp, 1337);
	close(server_sockfd);
}

void start_server(unsigned int port)
{
	printf("Starting server\n"); 

	int sockfd, new_fd; 
	struct addrinfo hints, *servinfo, *p; 
	struct sockaddr_storage their_addr; 
	socklen_t sin_size; 
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
			handle_client(new_fd); 

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

