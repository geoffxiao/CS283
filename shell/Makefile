.Phony : All Clean

All : tsh

tsh : tsh.c
	gcc tsh.c csapp.c -o tsh -pthread

Debug : tsh.c
	gcc -g tsh.c csapp.c -o tsh.o -pthread
	valgrind --track-origins=yes --leak-check=yes ./tsh.o

Clean:
	rm -f tsh tsh.o
