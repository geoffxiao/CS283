.PHONY : All
All : P1 P2 hello-hw dotpr-hw

P1 : P1.c csapp.c csapp.h
	gcc csapp.c P1.c -o P1 -pthread

P2 : P2.c csapp.c csapp.h
	gcc csapp.c P2.c -o P2 -pthread

hello-hw : hello-hw.c csapp.c csapp.h
	gcc csapp.c hello-hw.c -o hello-hw -pthread

dotpr-hw : dotpr-hw.c csapp.c csapp.h
	gcc csapp.c dotpr-hw.c -o dotpr-hw -pthread

Debug : dotpr-hw.c csapp.c csapp.h
	gcc -g dotpr-hw.c csapp.c -o dotpr-hw.o -pthread        
	valgrind --track-origins=yes --leak-check=yes ./dotpr-hw.o

