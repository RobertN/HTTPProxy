#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/queue.h>
#include "proxy.h"


int http_methods_len = 9; 
const char *http_methods[] = 
	{
		"OPTIONS", 
		"GET", 
		"HEAD", 
		"POST", 
		"PUT", 
		"DELETE", 
		"TRACE", 
		"CONNECT",
		"UNKNOWN"
	}; 

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



char *http_build_request(http_request *req)
{
	const char *search_path = req->search_path; 

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

	return request_buffer; 
}
