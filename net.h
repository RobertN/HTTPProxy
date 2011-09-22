#ifndef NET_HH
#define NET_HH

int http_connect(const char *host, const char *port); 
char *http_read_chunk(int sockfd);

#endif
