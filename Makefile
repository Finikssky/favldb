CC?=gcc
CFLAGS?=-std=gnu99 -g
LDFLAGS=-L.
IFLAGS=-I.

all: avl_lib

avl_lib:
	$(CC) $(CFLAGS) $(IFLAGS) -fPIC -c pool_allocator.c
	$(CC) $(CFLAGS) $(IFLAGS) -fPIC -c avl_tree.c
	$(CC) $(CFLAGS) -shared pool_allocator.o avl_tree.o -o libavl.so
	rm avl_tree.o

clean:
	rm *.so
	rm *.o