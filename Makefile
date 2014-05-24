SOURCES := $(shell find . -iname '*.c' -depth 1 )
OBJECTS := $(SOURCES:.c=.o)

all: http_proxy tests

http_proxy:	$(OBJECTS)
	gcc $(OBJECTS) $(LFLAGS) -o http_proxy

.PHONY:	tests
tests:
	gcc -I.. tests/http_message_test.c http_message.c -o http_message_test

.PHONY: clean
clean:
	rm $(OBJECTS) http_proxy
