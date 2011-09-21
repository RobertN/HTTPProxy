#include <sys/queue.h>
#include <stdio.h>
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
