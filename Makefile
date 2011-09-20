FILES = main.c http_message.c
all:
	gcc $(FILES) -Wall -g -o app
