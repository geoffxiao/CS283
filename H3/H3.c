#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_LINE_SIZE 2048
#define temp_file_name "./H3-temp-file"

void find_replace_all(char* original, char* find, char* replace, char* final);
void find_replace(char* original, char* find, char* replace, char* final);

int main(int argc, char** argv)
{

	// Check number of arguments
	if(argc != 4)
	{
		fprintf(stderr, "Invalid Number of Arguments\n");
		exit(1);
	}

	// Get arguments
	char* find = argv[1];
	char* replace = argv[2];
	char* prefix = argv[3];

	// Go through all .txt files in current directory
	// Will not recursively check the current directory
	DIR *directory = opendir("./"); // Directory
	struct dirent *file; // Current file

	while((file = readdir(directory)) != NULL)
	{
		
		
		// DT_REG = regular file
		// Also need to check that file extension is ".txt"
		int length = strlen(file->d_name);
		if(file->d_type == DT_REG && strncmp(file->d_name + length - 4, ".txt", 4) == 0)
		{
			FILE* original_file = fopen(file->d_name, "r");
			FILE* temp_file = fopen(temp_file_name, "w"); // temporary file

			int found_find = 0; // 0 -> no 'find' in 'original_file'

			char line[MAX_LINE_SIZE]; // a line in the file 'original_file'
			char to_write[MAX_LINE_SIZE];
			
			// Read entire file 'original_file'
			// replace 'find' parameter with the 'replace' parameter
			while(fgets(line, MAX_LINE_SIZE, original_file) != NULL)
			{
				if(strstr(line, find) != NULL)		
				{
					found_find = 1; // 1 -> 'find' is in the file
				}
				
				find_replace_all(line, find, replace, to_write); // replace 'find' with 'replace' in 'line'
				fprintf(temp_file, "%s", to_write);	// write to 'temp_file'
			}

			// close and reopen the files
			fclose(original_file); fclose(temp_file);
			original_file = fopen(file->d_name, "r"); 
			temp_file = fopen(temp_file_name, "w");

			// 'find' wasn't found in 'original_file'
			// find 'prefix' param, prepend 'find' param to the left of the 'prefix' param
			if(found_find == 0)
			{
				while(fgets(line, MAX_LINE,SIZE, original_file) != NULL)
				{
					prefix_prepend(line, prefix, find, to_write);	
				}
			
			}

			fclose(original_file); fclose(temp_file);

			// rename 'temp_file' to be the 'original_file' name
			remove(file->d_name);
			rename(temp_file_name, file->d_name);

		}
	}


	exit(0);

}


// Inputs: original = original string, "stuffA...find...stuffB"
//			  find = string to find, "find"
//         replace = string to replace find with, "replace"
//         final = first occurrence of find string is replaced with replace string, "stuffA...replace...stuffB"
// Replace first occurrence of find is replaced with replace
void find_replace(char* original, char* find, char* replace, char* final)
{
	char up_to_find[MAX_LINE_SIZE];

	// original = "stuffA...find...stuffB"

	// find_to_end = pointer to find...stuffB in original
	char* find_to_end = strstr(original, find); 


	// up_to_find = copy of stuffA...
	strcpy(up_to_find, original);
	up_to_find[strlen(original) - strlen(find_to_end)] = '\0'; // strlen(original) = strlen(find_to_end) + strlen(up_to_find)

	// past_find_to_end = pointer to ...stuffB
	char* past_find_to_end = original + strlen(up_to_find) + strlen(find);

	// copy stuffA...replace...stuffB into final
	strcpy(final, up_to_find); // final = stuffA...
	strcat(final, replace); // final = stuffA...replace
	strcat(final, past_find_to_end); // final = stuffA...replace...stuffB

}

// Inputs: original = original string, "stuffA...find...stuffB...find...stuffC..."
//         find = string to find, "find"
//         replace = string with which to replace find, "replae"
//         final = replaced all occurrences of find with replace, "stuffA...replace...stuffB...replace...stuffC"
//
// Replace all occurrences of find in the original string with the replace string
void find_replace_all(char* original, char* find, char* replace, char* final)
{
	char* exists; 
	char original_temp[MAX_LINE_SIZE]; // local copy of original 
	strcpy(original_temp, original);
	strcpy(final, original); // if find doesn't exist in original, final = original   
 
	// Keep running find_replace until find is no longer in the original_temp string
	while( (exists = strstr(original_temp,find)) != NULL)
	{
		find_replace(original_temp, find, replace, final);
		strcpy(original_temp, final); 
	}
}
