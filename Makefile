all: rpfs.c python_caller.o python_caller.h
	cc -libssl-dev python_caller.o rpfs.c -o python_caller

clean:
	rm python_caller
