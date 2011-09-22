FILES = main.c http_message.c net.c list.c
all:
	gcc $(FILES) -Wall -g -o app
