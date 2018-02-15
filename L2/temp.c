#include <stdio.h>
#define BLOCK_SIZE 512
int main(int argc, const char *argv[])
{
    char buffer[BLOCK_SIZE];
    for(;;) {
        size_t bytes = fread(buffer,  sizeof(char),BLOCK_SIZE,stdin);
        fwrite(buffer, sizeof(char), bytes, stdout);
        fflush(stdout);
        if (bytes < BLOCK_SIZE)
            if (feof(stdin))
                break;
    }    
	 return 0;
}
