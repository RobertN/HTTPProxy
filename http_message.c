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



char *http_build_request(http_request *req)
{
	const char *method = http_methods[req->method]; 
	const char *search_path = req->search_path; 

	int len = strlen(method) + 1 + strlen(search_path) +
		1 + strlen("HTTP/1.1") + strlen("\r\n") + 1; 
	char *request_buffer = malloc(sizeof(char)*len); 
	memset(request_buffer, sizeof(char)*len, '\0');

	snprintf(request_buffer, len, "%s %s HTTP/1.1\r\n", 
		method, 
		search_path); 

	len += strlen("Host: localhost\r\n\r\n"); 
	request_buffer = realloc(request_buffer, sizeof(char)*len); 

	strncat(request_buffer, "Host: localhost\r\n\r\n", len); 

	printf("Created HTTP request: \n%s\n", 
		request_buffer); 

	return request_buffer; 
}
