/*

		Take two arguments ./a and ./b.
		If a file in a does not exist in b, replicate the file in a into b
		If a file in b does not exist in a, it should be deleted from b
		If a file exists in both directories a and b, the file with the most
		recent modified date/time should be copied from one directory to the
		other
		Print a log of the program's activities to stdout or stderr

*/

// Copy file permissions too!!!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h> // For opendir
#include <unistd.h> 
#include <sys/stat.h> // For stat
#include <stdbool.h> // For bool type 
#include <time.h> // For time type

#define MAX_NUM_FILES 2048 // Maximum number of files in a directory

DIR* Open_Dir(char* dir_name);
int Get_Files(DIR* dir, char** files);
void Get_Full_Path(char* directory, char** files, int num_files, char** files_out);
int Get_Reg_Files(char* directory, char** files, int num_files);
bool File_Exists(char* file, char** array, int array_size);
int Files_To_Copy(char** from_dir_files,char* from_dir, int from_dir_size, 
					   char** to_dir_files, char* to_dir, int to_dir_size, 
						char** from_out, char** to_out);
void Copy_From_To(char* from, char* to);


int main(int argc, char** argv)
{

	if(argc != 3)
	{
		fprintf(stderr, "Invalid Number of Arguments\n");
		exit(1);
	}

	// Get directory names, check to make sure that they are directories
	char* dir_a_name = argv[1]; char* dir_b_name = argv[2];
	if( dir_a_name[strlen(dir_a_name) - 1] != '/' ||
	 	 dir_b_name[strlen(dir_b_name) - 1] != '/' )
	{
		fprintf(stderr, "Directory Not Passed\n");
		exit(1);
	}

	// Init variables
	DIR* dir_a; DIR* dir_b;
	char* dir_a_files[MAX_NUM_FILES]; char* dir_b_files[MAX_NUM_FILES];

	// Open directories
	dir_a = Open_Dir(dir_a_name); dir_b = Open_Dir(dir_b_name);

	// Get files in each directory
	int dir_a_num_files = Get_Files(dir_a, dir_a_files); 
	int dir_b_num_files = Get_Files(dir_b, dir_b_files);
	
	dir_a_num_files = Get_Reg_Files(dir_a_name, dir_a_files, dir_a_num_files);	
	dir_b_num_files = Get_Reg_Files(dir_b_name, dir_b_files, dir_b_num_files);	

	// Which files should we copy from a to b
	char* copy_from_a[MAX_NUM_FILES]; 	
	char* copy_to_b[MAX_NUM_FILES];
	int size_a_to_b = Files_To_Copy(dir_a_files, dir_a_name, dir_a_num_files, 
											  dir_b_files, dir_b_name, dir_b_num_files, 
											  copy_from_a, copy_to_b);

	// Which files should we copy from b to a
	char* copy_from_b[MAX_NUM_FILES]; 	
	char* copy_to_a[MAX_NUM_FILES];
	int size_b_to_a = Files_To_Copy(dir_b_files, dir_b_name, dir_b_num_files, 
											  dir_a_files, dir_a_name, dir_a_num_files,
											  copy_from_b, copy_to_a);

	// Synchronize files -- copy from directory a to directory b
	for(int i = 0; i < size_a_to_b; i++)
	{
		fprintf(stdout, "Copy from %s to %s\n", copy_from_a[i], copy_to_b[i]);
		Copy_From_To(copy_from_a[i], copy_to_b[i]);
	}

	// Synchronize files -- copy from directory b to directory a
	for(int i = 0; i < size_b_to_a; i++)
	{
		fprintf(stdout, "Copy from %s to %s\n", copy_from_b[i], copy_to_a[i]);
		Copy_From_To(copy_from_b[i], copy_to_a[i]);
	}

	exit(0);
}


// Copy file `from` to directory `to`
void Copy_From_To(char* from, char* to)
{
	FILE* from_file = fopen(from, "r");
	FILE* to_file = fopen(to, "w+");

	char buffer[256];
	int bytes_read;

	while(1)
	{
		size_t bytes_read = fread(buffer,  sizeof(char), sizeof(buffer), from_file);
		fwrite(buffer, sizeof(char), bytes_read, to_file);
		if(bytes_read < sizeof(buffer) && feof(from_file))
			break;
	}
			
}

int Files_To_Copy(char** from_dir_files, char* from_dir, int from_dir_size, 
					   char** to_dir_files, char* to_dir, int to_dir_size, 
						char** from_out, char** to_out)
{
	int c = 0;

	// Loop through `from_dir_files`
	for(int i = 0; i < from_dir_size; i++)
	{
		char* from = malloc( strlen(from_dir_files[i]) + strlen(from_dir) + 1);			
		from[0] = '\0';
		strcat(from, from_dir); 
		strcat(from, from_dir_files[i]);

		char* to = malloc( strlen(to_dir) + strlen(from_dir_files[i]) + 1 );
		to[0] = '\0';
		strcat(to, to_dir);
		strcat(to, from_dir_files[i]);
	
		// If file in `from` doesn't exist in `to`
		if( ! File_Exists(from_dir_files[i], to_dir_files, to_dir_size) )
		{
			from_out[c] = malloc( strlen(from) + 1 );
			strcpy(from_out[c], from);
	
			to_out[c] = malloc( strlen(to) + 1 );
			strcpy(to_out[c], to);

			c++;
		}
		else // File exists in `from` and `to`
		{
			struct stat from_file;
			stat(from, &from_file);

			struct stat to_file;
			stat(to, &to_file);

			// If `from_file` was modified earlier than `to_file`, copy `from`
			// to `to`
			if( difftime(from_file.st_mtime, to_file.st_mtime) > 0 )
			{
				from_out[c] = malloc( strlen(from) + 1 );
				strcpy(from_out[c], from);
				
				to_out[c] = malloc( strlen(to) + 1 );
				strcpy(to_out[c], to);
	
				c++;
			}			
		}
		free(to); free(from);
	}
	return c;
}


bool File_Exists(char* file, char** array, int array_size)
{
	for(int i = 0; i < array_size; i++)
	{
		if(strcmp(file, array[i]) == 0)
			return true;
	}
	return false;
}

// Only get the regular files in the `files` array. Return number of regular
// files
int Get_Reg_Files(char* directory, char** files, int num_files)
{
	char* tmp;
	struct stat check_file;
	int c = 0;

	for(int i = 0; i < num_files; i++)
	{
		tmp = malloc( strlen(directory) + strlen(files[i]) + 1 );
		tmp[0] = '\0';
		strcat(tmp, directory);
		strcat(tmp, files[i]);
		stat(tmp, &check_file);	
	
		// Only care about regular files
		if( S_ISREG(check_file.st_mode) )
		{
			char* tmp2 = malloc( strlen(files[i]) + 1 );
			strcpy(tmp2, files[i]);
			free(files[c]);
			files[c] = malloc( strlen(files[i]) + 1);
			strcpy(files[c], tmp2);
			free(tmp2);
			c++;
		}
		free(tmp);
	}
	return c;
}


// Get the full path name of the files in `files`. The `files` live in
// directory `directory`. Store full path names in `files_out`
void Get_Full_Path(char* directory, char** files, int num_files, char** files_out)
{
	char* tmp;
	struct stat check_file;

	// Get full path names
	// Store in files_out
	for(int i = 0; i < num_files; i++)
	{
		tmp = malloc( strlen(directory) + strlen(files[i]) + 1 );
		tmp[0] = '\0';
		strcat(tmp, directory);
		strcat(tmp, files[i]);
		stat(tmp, &check_file);	
	
		files_out[i] = malloc( strlen(tmp) + 1 );
		strcpy(files_out[i], tmp); // Change file names to full path name
		free(tmp);
	}
}


// Get all files in `DIR* dir`, save file names in `files` array
int Get_Files(DIR* dir, char** files)
{
	int c = 0;
	struct dirent* directory_ent;
	while( (directory_ent = readdir(dir)) != NULL )
	{
		files[c] = malloc( sizeof(directory_ent->d_name) );
		strcpy(files[c], directory_ent->d_name);
		c++;
	}
	return c; // Number of files
}


// Return `DIR*` (directory stream) for specified `dir_name`
DIR* Open_Dir(char* dir_name)
{
	DIR* dir;
	if( (dir = opendir(dir_name)) == NULL )
	{
		fprintf(stderr, "Failed to Open Directory %s\n", dir_name);
		exit(1);
	}
	return dir;
}
