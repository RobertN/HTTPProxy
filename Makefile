SOURCES := $(shell find . -iname '*.c' )
OBJECTS := $(SOURCES:.c=.o)

all: http_proxy

http_proxy:	$(OBJECTS)
	gcc $(OBJECTS) $(LFLAGS) -o http_proxy

.PHONY: clean
clean:
	rm $(OBJECTS) http_proxy
