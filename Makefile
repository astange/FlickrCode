all: python_caller.c
	cc -I python_caller.c -o python_caller

clean:
	rm python_caller
