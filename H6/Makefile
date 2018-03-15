.PHONY : All
All : P1 P2 P3

P1 : P1.c csapp.c
	gcc csapp.c P1.c -o P1 -pthread

P2 : P2.c csapp.c
	gcc csapp.c P2.c -o P2 -pthread

P3 : P3.c csapp.c
	gcc csapp.c P3.c -o P3 -pthread

Debug : P3.c
	gcc -g P3.c csapp.c -o P3.o -pthread        
	valgrind --track-origins=yes --leak-check=yes ./P3.o
