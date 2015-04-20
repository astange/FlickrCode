all: rpfs.c python_caller.c python_caller.h rpfs.h
	cc python_caller.c rpfs.c -o rpfs -D_FILE_OFFSET_BITS=64 -I/usr/local/include/fuse  -pthread -L/usr/local/lib -lfuse -lssl -lcrypto

clean:
	rm rpfs
