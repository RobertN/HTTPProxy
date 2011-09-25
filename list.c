#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "proxy.h"

const char *list_get_key(struct METADATA_HEAD *list, const char *key)
{
	http_metadata_item *item; 
	TAILQ_FOREACH(item, list, entries) {
		if(strcmp(item->key, key) == 0)
		{
			return item->value; 
		}
	}

	return NULL;
}

void list_add_key(struct METADATA_HEAD *list, const char *key, const char *value)
{
	http_metadata_item *item = (http_metadata_item*)malloc(sizeof(http_metadata_item));
	item->key = key; 
	item->value = value; 

	TAILQ_INSERT_TAIL(list, item, entries); 
}
