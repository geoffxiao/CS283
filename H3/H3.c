#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_LINE_SIZE 2048
#define temp_file_name "./H3-temp-file"

// find_replace_all: Replace all occurrences of `find` in `original` with `replace`. Store
//                   the result in `final`. Use `log_str` to keep track of the number of 
//                   changes and the location of changes in `final`.
void find_replace_all(char* original, char* find, char* replace, char* final, char* log_str);
int find_replace(char* original, char* find, char* replace, char* final, int start);


// prefix_prepend_all: For all instances of `prefix` in `original`, prepend `find` to the left of
//                     `find`. Store the result in `final`. Use `log_str` to keep track of the 
//                     number of changes and the location of changes in `final`.
void prefix_prepend_all(char* original, char* prefix, char* find, char* final, char* log_str);
int prefix_prepend(char* original, char* prefix, char* find, char* final, int start);

// argv: 1 = find
//	     2 = replace
//       3 = prefix
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
			fprintf(stdout, "-----Processing File: %s-----\n", file->d_name);

			FILE* original_file = fopen(file->d_name, "r");
			FILE* temp_file = fopen(temp_file_name, "w"); // temporary file

			int found_find = 0; // 0 -> no 'find' in 'original_file'

			char line[MAX_LINE_SIZE]; // a line in the file 'original_file'
			char to_write[MAX_LINE_SIZE]; // processed line to write to `temp_file`
			
			char log_str[MAX_LINE_SIZE]; // changes made to `to_write` from `line`
			log_str[0] = '\0';

			int line_num = 1; // current line in `original_file`
			
			// Read entire file 'original_file'
			// replace 'find' parameter with the 'replace' parameter
			while(fgets(line, MAX_LINE_SIZE, original_file) != NULL)
			{
				if(found_find == 0 && strstr(line, find) != NULL)		
				{
					found_find = 1; // 1 -> 'find' is in the file
				}
				
				find_replace_all(line, find, replace, to_write, log_str); // replace 'find' with 'replace' in 'line'
				fprintf(temp_file, "%s", to_write);	// write to 'temp_file'

				// print `log_str` to stdout
				fprintf(stdout, "Line %d: %s\n", line_num, log_str);
				line_num++;
			}

			// close and reopen the files
			fclose(original_file); fclose(temp_file);
			// rename 'temp_file' to be the 'original_file' name
			remove(file->d_name);
			rename(temp_file_name, file->d_name);			


			// 'find' wasn't found in 'original_file'
			// find 'prefix' param, prepend 'find' param to the left of the 'prefix' param
			if(found_find == 0)
			{
				line_num = 1;	

				// open the files
				temp_file = fopen(temp_file_name, "w"); // temporary file
				original_file = fopen(file->d_name, "r");			

				while(fgets(line, MAX_LINE_SIZE, original_file) != NULL)
				{
					prefix_prepend_all(line, prefix, find, to_write, log_str);
					fprintf(temp_file, "%s", to_write); // write to 'temp_file'

					// print 'log_str' to stdout
					fprintf(stdout, "Line %d: %s\n", line_num, log_str);
					line_num++;
				}
			
				fclose(original_file); fclose(temp_file);
				// rename 'temp_file' to be the 'original_file' name
				remove(file->d_name);
				rename(temp_file_name, file->d_name);
			}
		}
	}

	exit(0);
}


// -----------------------------------------------------------------------------------------------------------
// Helper Functions
// -----------------------------------------------------------------------------------------------------------


// Inputs: original = original string of the form, "stuffA...prefix...stuffB...prefix...stuffC..."
//         prefix = `prefix` string
//         find = `find` string, will prepend `find` to the left of all instances of `prefix`
//         start = integer to keep track of current location in original string
//         final = where to store the output string
//               = "stuffA...findprefix...stuffB...prefix...stuffC"
//
// Starting at the `start` char of `original`, find the first instance of `prefix` and prepend `find` to the 
// left of `prefix`
int prefix_prepend(char* original, char* prefix, char* find, char* final, int start)
{
    // `original` = "stuffA...prefix...stuffB...prefix...stuffC"

    char up_to_prefix[MAX_LINE_SIZE];
    
    char* start_str = &original[start]; // start from `start` byte
    char* prefix_to_end = strstr(start_str, prefix);  // `prefix_to_end` = pointer to "prefix...stuffB...prefix...stuffC" in `original`
    
    // if `start` is past first case of `prefix`
    //    `prefix_to_end` = "prefix...stuffC"

    // `up_to_prefix` = copy of "stuffA..." 
    // strlen(original) = strlen(prefix_to_end) + strlen(up_to_prefix)
    strcpy(up_to_prefix, original);
    up_to_prefix[strlen(original) - strlen(prefix_to_end)] = '\0'; 
    
    // copy "stuffA...findprefix...stuffB...prefix...stuffC" into `final`
    strcpy(final, up_to_prefix); // `final` = "stuffA..."
    strcat(final, find); // `final` = "stuffA...find"
    strcat(final, prefix_to_end); // `final` = "stuffA...findprefix...stuffB...prefix...stuffC"

    return strlen(up_to_prefix) + strlen(find) + strlen(prefix); // return location of first char after "findprefix"
}


// Inputs: original = original string of the form, "stuffA...prefix...stuffB...prefix...stuffC..."
//         prefix = `prefix` string
//         find = `find` string, will prepend `find` to the left of all instances of `prefix`
//         final = where to store the output string
//               = "stuffA...findprefix...stuffB...findprefix...stuffC"
//         log_str = keeps track of number of changes made and where the changes are in `original`
//
// Find all instances of `prefix` in `original` and prepend `find` to the left of `prefix`
void prefix_prepend_all(char* original, char* prefix, char* find, char* final, char* log_str)
{
    char original_temp[300]; // local copy of `original`
    strcpy(original_temp, original);
    
    strcpy(final, original); // `final` = the processed string
    
    int curr_loc = 0; // current location in `original_temp`
    char* start_str = &original_temp[curr_loc]; // only look to the left of `curr_loc` byte in `original_temp`
    
    int num_changes = 0; // number of changes made
    strcpy(log_str, "at character(s) "); // initialize
    
    // Keep running `prefix_prepend` until `prefix` is no longer in the `start_str` string
    while( strstr(start_str, prefix) != NULL )
    {
        curr_loc = prefix_prepend(original_temp, prefix, find, final, curr_loc); // update current location in `original_temp`
        strcpy(original_temp, final); // update `original_temp`
        start_str = &original_temp[curr_loc]; // update `start_str`

        // Update log string
        num_changes++;
        char local_temp[MAX_LINE_SIZE];
        sprintf(local_temp, " %ld", curr_loc - strlen(prefix) - strlen(find) + 1); // +1 because char array starts at 0 index
        strcat(log_str, local_temp);
    }
    
    // Create log string
    char temp_str[MAX_LINE_SIZE];
    sprintf(temp_str, "%d prepend(s) made ", num_changes);
    strcat(temp_str, log_str);
    strcpy(log_str, temp_str);    

	 if(num_changes == 0)
		 strcpy(log_str, "0 prepends made");
}


// Inputs: original = original string, "stuffA...find...stuffB...find...stuffC"
//		   find = string to find, "find"
//         replace = string to replace find with, "replace"
//         start = integer to keep track of current location in original string
//         final = where to store processed string
//               = first occurrence of find string is replaced with replace string, "stuffA...replace...stuffB...find...stuffC"
//
// Replace first occurrence of `find` with `replace` in `original`
int find_replace(char* original, char* find, char* replace, char* final, int start)
{
    // `original` = "stuffA...find...stuffB...find...stuffC"

    char up_to_find[MAX_LINE_SIZE];
   
    char* start_str = &original[start]; // start from `start` byte
    char* find_to_end = strstr(start_str, find); // `find_to_end` = pointer to "find...stuffB...find...stuffC" in `original`

    // `up_to_find` = copy of "stuffA..."
    // strlen(original) = strlen(find_to_end) + strlen(up_to_find)
    strcpy(up_to_find, original);
    up_to_find[strlen(original) - strlen(find_to_end)] = '\0';
    
    // `past_find_to_end` = pointer to "...stuffB...find...stuffC"
    char* past_find_to_end = original + strlen(up_to_find) + strlen(find);

    // copy "stuffA...replace...stuffB...find...stuffC" into final
    strcpy(final, up_to_find); // `final` = "stuffA..."
    strcat(final, replace); // `final` = "stuffA...replace"
    strcat(final, past_find_to_end); // `final` = "stuffA...replace...stuffB...find...stuffC"

    return strlen(up_to_find) + strlen(replace); // return location of first char after 'replace'
}


// Inputs: original = original string, "stuffA...find...stuffB...find...stuffC"
//		   find = string to find, "find"
//         replace = string to replace find with, "replace"
//         final = where to store processed string
//               = all occurrences of find string is replaced with replace string, "stuffA...replace...stuffB...replace...stuffC"
//         log_str = keeps track of the number of changes and where the changes were made in `final`
//
// Replace all occurrences of `find` with `replace`
void find_replace_all(char* original, char* find, char* replace, char* final, char* log_str)
{
    char original_temp[MAX_LINE_SIZE]; // local copy of `original`
    strcpy(original_temp, original);
    
    strcpy(final, original); // `final` = store processed string
    
    int curr_loc = 0; // current location in `original_temp`
    char* start_str = &original_temp[curr_loc]; // look to the left of `curr_loc` in `original_temp` for `find`
    
    int num_changes = 0; // number of changes made
    strcpy(log_str, "at character(s) "); // initialize
    
    // Keep running `find_replace` until find is no longer in the `start_str` string
    while( strstr(start_str, find) != NULL)
    {
        curr_loc = find_replace(original_temp, find, replace, final, curr_loc); // update `curr_loc`
        strcpy(original_temp, final); // update `original_temp`
        start_str = &original_temp[curr_loc]; // update `start_str`

        // Update log string
        num_changes++;
        char local_temp[MAX_LINE_SIZE];
        sprintf(local_temp, " %ld", curr_loc - strlen(replace) + 1); // +1 because char array starts at 0 index
        strcat(log_str, local_temp);
    }
    
    // Create log string
    char temp_str[MAX_LINE_SIZE];
    sprintf(temp_str, "%d replacements(s) made ", num_changes);
    strcat(temp_str, log_str);
    strcpy(log_str, temp_str);

	 if(num_changes == 0)
		 strcpy(log_str, "0 replacements made");
}