.Phony : All Clean

All : L2

L2 : L2.c
	gcc L2.c -o L2

Debug : L2.c
	gcc -g L2.c -o L2.o
	valgrind --leak-check=yes ./L2.o ./ ./

Clean:
	rm -f L2 L2.o
