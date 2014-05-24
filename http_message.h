#ifndef HTTP_MESSAGE_HH
#define HTTP_MESSAGE_HH
#include "proxy.h"

void http_request_init(http_request**);
void http_request_destroy(http_request*);
void http_request_print(http_request*);
void http_parse_method(http_request*, char*);
void http_parse_metadata(http_request*, char*);
char *http_build_request(http_request*); 

extern int http_methods_len;
extern const char* http_methods[];

#endif
