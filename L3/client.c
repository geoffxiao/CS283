/*
 * Geoffrey Xiao
 * gx26
 * 
 * Lab 3 - Client
 *
 * client <host> <port> <file>
 * 	Send request to server
 *
 */
#include "csapp.h"

int main(int argc, char **argv) 
{
	// Invalid Number of Arguments
	if(argc != 4)
	{
		fprintf(stderr, "usage: %s <host> <port> <file>\n", argv[0]);
		exit(1); // ERROR exit
	}

	// Get Arguments
	char* host = argv[1];
	int port = atoi(argv[2]); 
	char* file = argv[3];

	int clientfd = Open_clientfd(host, port); // Open client connection

	// Send Request to Sever
	char* send_request = malloc( 
			strlen("GET ") + strlen(file) + strlen(" HTTP/1.1\r\n") +
			strlen("Host: ") + strlen(host) + strlen("\r\n") + 
			strlen("\r\n") + 1);
	strcpy(send_request, "GET ");
  	strcat(send_request, file);
	strcat(send_request, " HTTP/1.1\r\n");
	strcat(send_request, "Host: ");
	strcat(send_request, host);
	strcat(send_request, "\r\n");
	strcat(send_request, "\r\n");	

	// Print to Client
	Fputs(send_request, stdout);
	
	// Send Request to Server
	Rio_writen(clientfd, send_request, strlen(send_request));

	// Read from Server Socket
	rio_t rio;
	Rio_readinitb(&rio, clientfd);
	char buf[MAXBUF];
	ssize_t nread;
	while((nread = Rio_readnb(&rio, buf, MAXLINE)) != 0) // Keep reading until EOF
	{
		Fputs(buf, stdout);
	}
	if(nread < 0)
	{
		fprintf(stderr, "Error Reading from Server\n");
	}

	free(send_request);
	Close(clientfd);
}
