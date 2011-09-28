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
    LOG(LOG_TRACE, "http_request_destroy\n");
    free((char*)req->search_path);

    struct http_metadata_item *item; 
    TAILQ_FOREACH(item, &req->metadata_head, entries) {
        free((char*)item->key);
        free((char*)item->value); 
        free(item);
    }
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
    char *line_copy = strdup(line); 
    char *str_part = strtok(line_copy, " ");


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

    free(line_copy);

    // Get the search path from the request
    // (perhaps this should only be done when we
    // get a GET request?)

    char *p = line; 
    int state = 0; 
    while(1)
    {
        if(*p == ' ' && state == 0)
        {
            // start parsing the host 
            state = 1; 
        } else if(*p == '/' && (state == 1 || state == 2))
        {
            // read after http://             
            state++;
        } else if(*p == '/' && state == 3)
        {
            // we have come to the final /
            break; 
        }

        p++; 
    }
   
    str_part = strtok(p, " "); 
    printf("str_part: %s\n", str_part);
    result->search_path = strdup(str_part); 


    // TODO: Retrieve the HTTP version
    str_part = strtok(NULL, "\r");
    if(strcmp(str_part, "HTTP/1.0") == 0)
    {
        result->version = HTTP_VERSION_1_0;
    } else if(strcmp(str_part, "HTTP/1.1") == 0)
    {
        result->version = HTTP_VERSION_1_1;
    } else
    {
        LOG(LOG_WARNING, "Unknown HTTP version\n");
    }
}

// Content-Byte: 101
void http_parse_metadata(http_request *result, char *line)
{
    char *line_copy = strdup(line); 
    char *key = strdup(strtok(line_copy, ":")); 

    char *value = strtok(NULL, "\r"); 

    // remove whitespaces :)
    char *p = value; 
    while(*p == ' ') p++; 
    value = strdup(p); 

    free(line_copy);

    // create the http_metadata_item object and
    // put the data in it
    http_metadata_item *item = malloc(sizeof(*item)); 
    item->key = key; 
    item->value = value; 

    // add the new item to the list of metadatas
    TAILQ_INSERT_TAIL(&result->metadata_head, item, entries); 
}



char *http_build_request(http_request *req)
{
    const char *search_path = req->search_path; 

    // construct the http request 
    int size = strlen("GET ") + 1; 
    //char *request_buffer = calloc(sizeof(char)*size);
    char *request_buffer = calloc(size, sizeof(char));
    strncat(request_buffer, "GET ", 4);

    size += strlen(search_path) + 1; 
    request_buffer = realloc(request_buffer, size);
    strncat(request_buffer, search_path, strlen(search_path));

    // TODO: Check the actual HTTP version that is used, and if 
    // 1.1 is used we should append:
    // 	Connection: close 
    // to the header. 
    switch(req->version)
    {
        case HTTP_VERSION_1_0:
            size += strlen(" HTTP/1.0\r\n\r\n");
            request_buffer = realloc(request_buffer, size); 
            strncat(request_buffer, " HTTP/1.0\r\n", strlen(" HTTP/1.0\r\n"));
            break; 
        case HTTP_VERSION_1_1:
            size += strlen(" HTTP/1.1\r\n\r\n");
            request_buffer = realloc(request_buffer, size); 
            strncat(request_buffer, " HTTP/1.1\r\n", strlen(" HTTP/1.1\r\n"));
            break; 
        default: 
            LOG(LOG_ERROR, "Failed to retrieve the http version\n");
            return NULL; 
    }

    http_metadata_item *item; 
    TAILQ_FOREACH(item, &req->metadata_head, entries) {
        // Remove Connection properties in header in case
        // there are any
        if(strcmp(item->key, "Connection") == 0 || 
            strcmp(item->key, "Proxy-Connection") == 0)
        {
            continue; 
        }

        size += strlen(item->key) + strlen(": ") + strlen(item->value) + strlen("\r\n");  
        request_buffer = realloc(request_buffer, size);
        strncat(request_buffer, item->key, strlen(item->key)); 
        strncat(request_buffer, ": ", 3);
        strncat(request_buffer, item->value, strlen(item->value));
        strncat(request_buffer, "\r\n", 2);
    }

    if(req->version == HTTP_VERSION_1_1)
    {
        size += strlen("Connection: close\r\n");
        request_buffer = realloc(request_buffer, size);
        strncat(request_buffer, "Connection: close\r\n", strlen("Connection: close\r\n"));
    }


    size += strlen("\r\n");
    request_buffer = realloc(request_buffer, size);
    strncat(request_buffer, "\r\n", 2);

    return request_buffer; 
}
