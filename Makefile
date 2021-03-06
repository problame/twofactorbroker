CC ?= gcc
CFLAGS = -g -O0 -c -std=gnu99

tfybk: main.o common.o broker.o client.o yubikey.o dummytransformer.o passphrase_logic.o util.o provision.o salt.o
	$(CC) -lreadline -lykpers-1 -lgcrypt \
	      -o tfybk \
		  main.o common.o \
		  broker.o client.o provision.o salt.o \
		  yubikey.o dummytransformer.o passphrase_logic.o util.o 
%.o: $.c
	$(CC) $(CFLAGS) -o $@ $<
