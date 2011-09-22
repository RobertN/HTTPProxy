#ifndef PROXY_HH
#define PROXY_HH

#define LOG_ERROR		0
#define LOG_WARNING 1
#define LOG_NOTICE 	2
#define LOG_TRACE		3

#define ACTIVE_LEVEL 3
#define LOG(LEVEL, MSG) 					\
	if(LEVEL <= ACTIVE_LEVEL)  {		\
		printf("LOG(%d): ", LEVEL); 	\
		printf(MSG);									\
	}																\

extern int http_methods_len; 
extern const char *http_methods[];  

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

typedef struct http_request
	{
		enum http_methods_enum method; 
		const char *search_path; 

		TAILQ_HEAD(METADATA_HEAD, http_metadata_item) metadata_head; 
	} http_request;

typedef struct http_metadata_item
	{ 
		const char *key; 
		const char *value; 
		
		TAILQ_ENTRY(http_metadata_item) entries; 
	} http_metadata_item; 


#endif
