.Phony: All

All : client server

client : client.c
	gcc client.c csapp.c -o client -pthread

server : server.c
	gcc server.c csapp.c -o server -pthread

