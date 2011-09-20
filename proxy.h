#ifndef PROXY_HH
#define PROXY_HH

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

typedef struct
	{
		enum http_methods_enum method; 
		const char *search_path; 

		TAILQ_HEAD(, tailq_entry) metadata_head; 
	} http_request;

typedef struct
	{ 
		const char *key; 
		const char *value; 
		
		TAILQ_ENTRY(tailq_entry) entries; 
	} http_metadata_item; 


#endif

