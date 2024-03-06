CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra

nyush: nyush.c
	$(CC) $(CFLAGS) -o nyush nyush.c

.PHONY: clean
clean:
	rm -f nyush
