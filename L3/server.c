/*
 * Geoffrey Xiao
 * 
 * CS283 Lab 3 - Server
 *
 * server <port>
 *		Establish server at port number. 
 * 	Server constantly accepts connetions
 *
 */
#include "csapp.h"

void doit(int fd); // Process a HTTP request

void read_requesthdrs(rio_t *rp); // Process request headers

void parse_uri(char *uri, char *filename, char *cgiargs); // Process URI to filename

void clienterror(int fd, char *cause, char *errnum, // Error Message
					  char *shortmsg, char *longmsg);

// main
int main(int argc, char **argv) 
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
	 char clienthostname[MAXLINE], clientport[MAXLINE];

	 // Invalid Number of Arguments
	 if (argc != 2) 
	 {
		 fprintf(stderr, "usage: %s <port>\n", argv[0]);
		 exit(1);
	 }

	 // Open server listening connection
    port = atoi(argv[1]);
    listenfd = Open_listenfd(port);
	 
	 // listenfd persists
	 // allow server to constantly accept connections
	 while (1) 
	 {
		 // Get Client Request
		 clientlen = sizeof(clientaddr);
		 connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		 getnameinfo((SA*) &clientaddr, clientlen, clienthostname, MAXLINE,
						 clientport, MAXLINE, 0);
		 fprintf(stdout, "===================\nConnected to %s:%s\n", 
					clienthostname, clientport);
		 doit(connfd); // Run the HTTP request
		 Close(connfd);
    }
}

// doit - handle one HTTP request/response transaction
void doit(int fd) 
{
	struct stat sbuf;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];
	rio_t rio;
  
	// Read request line and headers
	Rio_readinitb(&rio, fd);
	Rio_readlineb(&rio, buf, MAXLINE);
	sscanf(buf, "%s %s %s", method, uri, version);
    
	// Only Supports GET
	if (strcasecmp(method, "GET")) 
	{
		clienterror(fd, method, "501", "Not Implemented",
				 		"Tiny does not implement this method");
	  	return;
	}
    
	read_requesthdrs(&rio); // Ignore All Request Headers
	parse_uri(uri, filename, cgiargs); // Get File to Send Back
	if (stat(filename, &sbuf) < 0) // File doesn't exist
	{
		clienterror(fd, filename, "404", "Not found",
						"Tiny couldn't find this file");
		fprintf(stdout, "Server Could Not Find %s\n\n", filename);
		return;
	}
	
	// file size	
	int filesize = sbuf.st_size;

	// Send Server Info
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer\r\n", buf);
	sprintf(buf, "%sContent-length %d\r\n", buf, filesize);
	sprintf(buf, "%sContent-type: \r\n", buf);
	sprintf(buf, "%s=========================\r\n\r\n", buf);
	Rio_writen(fd, buf, strlen(buf));

	// Get the File to Send
	int file_to_send = Open(filename, O_RDONLY, 0);
	char* buf_to_send = malloc(filesize);
	
	// Read From File
	rio_t rio_send_file;
	Rio_readinitb(&rio_send_file, file_to_send);
	Rio_readnb(&rio_send_file, buf_to_send, filesize);
	Close(file_to_send);

	// Write File to Client
	Rio_writen(fd, buf_to_send, filesize);

	fprintf(stdout, "Server Sent %s to Client\n\n", filename);

	free(buf_to_send);
}

// Process the Request Headers - Ignore All Request Headers
void read_requesthdrs(rio_t *rp) 
{
	char buf[MAXLINE];
	Rio_readlineb(rp, buf, MAXLINE);
	 
	while(strcmp(buf, "\r\n")) 
	{
		Rio_readlineb(rp, buf, MAXLINE);
		printf("%s", buf);
	}
    
	return;
}


// Parse the URI to a filename
// / -> ./index.html
void parse_uri(char *uri, char *filename, char *cgiargs) 
{
	strcpy(filename, ".");
	strcat(filename, uri);
	if(uri[strlen(uri) - 1] == '/')
	    strcat(filename, "index.html");
}


// Error Message
void clienterror(int fd, char *cause, char *errnum,
					  char *shortmsg, char *longmsg) 
{
    
	char buf[MAXLINE], body[MAXBUF];

	/* Build the HTTP response body */
	sprintf(body, "<html><title>Tiny Error</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    
	/* Print the HTTP response */
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
	Rio_writen(fd, buf, strlen(buf));
	Rio_writen(fd, body, strlen(body));
}
