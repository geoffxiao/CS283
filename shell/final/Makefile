.Phony : All Test Clean Debug

All : tsh

tsh : tsh.c
	gcc tsh.c csapp.c -o tsh -pthread

Debug : tsh.c
	gcc -g tsh.c csapp.c -o tsh.o -pthread
	valgrind --track-origins=yes --leak-check=yes ./tsh.o

Test : myint.c mystop.c mysplit.c myspin.c
	gcc myint.c -o myint
	gcc mystop.c -o mystop
	gcc mysplit.c -o mysplit
	gcc myspin.c -o myspin

Clean:
	rm -f tsh tsh.o
