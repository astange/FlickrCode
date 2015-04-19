all: rpfs.c python_caller.o python_caller.h
	cc -D_FILE_OFFSET_BITS=64 -I/usr/local/include/fuse  -pthread -L/usr/local/lib -lfuse -libssl-dev python_caller.o rpfs.c -o python_caller

python_caller.o : python_caller.c python_caller.h
        cc -c python_caller.c

clean:
	rm python_caller
