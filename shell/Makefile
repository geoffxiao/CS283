.Phony : All Clean

All : tsh

tsh : tsh.c
	gcc tsh.c -o tsh

Debug : tsh.c
	gcc -g tsh.c -o tsh.o
	valgrind --leak-check=yes ./tsh.o

Clean:
	rm -f tsh tsh.o
