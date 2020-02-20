all:mysh.c
	gcc mysh.c -o mysh.out
clean:
	rm -f mysh.out