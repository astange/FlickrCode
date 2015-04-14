all: python_caller.c
	gcc -I /usr/include/python2.7 python_caller.c -lpython2.7 -o python_caller

clean:
	rm python_caller
