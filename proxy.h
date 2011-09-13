#ifndef PROXY_HH
#define PROXY_HH

const int http_methods_len = 9; 
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

enum http_methods_enum 
	{
		OPTIONS,
		GET,
		HEAD,
		POST,
		PUT, 
		DELETE, 
		TRACE,
		CONNECT, 
		UNKNOWN
	};

typedef struct
	{
		enum http_methods_enum method; 
		const char *search_path; 
	} http_request;


#endif
