all: rpfs.c python_caller.o python_caller.h
	cc -std=c99 -D_FILE_OFFSET_BITS=64 -I/usr/local/include/fuse  -pthread -L/usr/local/lib -lfuse -libssl-dev python_caller.o rpfs.c -o python_caller

clean:
	rm python_caller
