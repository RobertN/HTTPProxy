FILES = main.c http_message.c list.c net.c
all:
	gcc $(FILES) -Wall -g -o app
