all: rpfs.c python_caller.c python_caller.h
	cc -D_FILE_OFFSET_BITS=64 -I/usr/local/include/fuse  -pthread -L/usr/local/lib -lfuse -lssl -lcrypto python_caller.c rpfs.c -o python_caller

clean:
	rm python_caller
