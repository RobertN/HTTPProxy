#ifndef NET_HH
#define NET_HH

int http_connect(http_request *req); 
char *http_read_chunk(int sockfd);

#endif
