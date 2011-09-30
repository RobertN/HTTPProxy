OS = `uname -a`

LFLAGS = 

ifeq ($(OS),Sun)
	LFLAGS = -lnsl -lsocket -lresolv
endif

FILES = main.c http_message.c list.c net.c

all:
	@echo Building for [$(OS)]
	gcc $(FILES) -Wall -g -o NetNinny 
