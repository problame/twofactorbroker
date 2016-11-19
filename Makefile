CC ?= gcc
CFLAGS = -g -O0 -c -std=gnu99

tfybk: main.o common.o broker.o client.o
	$(CC) -lreadline -o tfybk main.o common.o broker.o client.o

%.o: $.c
	$(CC) $(CFLAGS) -o $@ $<
