CC = gcc
CFLAGS = -fPIC -std=gnu99 -Wall -Wextra -O2
LDFLAGS = -shared
Valkey_MODULE_TARGET = aclreplication.so
Valkey_CFLAGS = -I.

all:
	$(CC) $(CFLAGS) $(Valkey_CFLAGS) -c aclreplication.c -o aclreplication.o
	$(CC) $(LDFLAGS) -o $(Valkey_MODULE_TARGET) aclreplication.o

clean:
	rm -f *.o $(Valkey_MODULE_TARGET)
    