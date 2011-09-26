#ifndef NET_HH
#define NET_HH

int http_connect(http_request *req); 
http_request *http_read_header(int sockfd);
char *http_read_chunk(int sockfd, ssize_t *length);

#endif
