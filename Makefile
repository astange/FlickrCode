all: python_caller.c
	cc -lpython2.7 -o python_caller python_caller.c

clean:
	rm python_caller