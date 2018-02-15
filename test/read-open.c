#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{

	char* filename = argv[1];

	int file_rd = open(filename, O_RDONLY);
	int file_wr = open(filename, O_WRONLY | O_APPEND);

	char read_buf[256]; char write_buf[256];
	strcpy(write_buf, "123456789abcdefg");

	int write_n = write(file_wr, write_buf, strlen(write_buf));
	int read_n = read(file_rd, read_buf, 256);
	read_buf[read_n] = '\0';

	printf("wrote %d\n", write_n);
	printf("read %d\n", read_n);
	printf("%s\n", read_buf);

	close(file_rd); close(file_wr);
	return 0;
}
