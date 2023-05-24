CC := gcc
CFLAGS := -Wall -Wextra -Wno-unknown-pragmas -Wpedantic
CFLAGS += -g
CFLAGS += -std=c17
CFLAGS += -O3
LDFLAGS :=

main: main.o
	$(CC) $^ $(LDFLAGS) -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -rf main main.o
