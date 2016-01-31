CC = gcc
LD = gcc

CPPFLAGS += -MD -MP -g -Werror 

SRC = $(wildcard src/*.c) $(wildcard src/uthash/*.c)

.PHONY: directories

all: directories sf

directories:
	mkdir -p Build/
	mkdir -p Build/src/

clean:
	rm -Rf Build/

sf: $(SRC:%.c=Build/%.o)
	$(LD) -o Build/$@ $^
	cp Build/$@ $@

Build/%.o: %.c
	$(CC) $(CPPFLAGS) -c -o $@ $^

-include $(SRC:%.c=%.d)

