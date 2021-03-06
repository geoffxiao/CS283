/*
	CS283 L2
	Geoffrey Xiao

	Take two arguments ./a and ./b.
	If a file in a does not exist in b, replicate the file in a into b
	If a file in b does not exist in a, it should be deleted from b
	If a file exists in both directories a and b, the file with the most
	recent modified date/time should be copied from one directory to the
	other
	
	Print a log of the program's activities to stdout or stderr

	Program copies file permissions
	
	Directory must end in a backslash just like command line `cp`

*/




// Needed for nftw
#define _XOPEN_SOURCE 700

// Maximum number of directories nftw will open simultaneously
#ifndef USE_FDS
#define USE_FDS 20
#endif

#include <ftw.h>
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

// Given an array `array` of size `array_size`, does `file` exist in `array`?
//
// Return True if `file` exists in `array`. False otherwise.
bool File_Exists(char* file, char** array, int array_size);

// Given a `dir` and a `file`, give the full pathname which is `dir``file`
// `dir` has a '\' already
//
// Return pointer to dynamically allocated memory storing `dir` concatenated
// with `file
char* Full_Path(char* dir, char* file);

// Delete the file specified by `file`. `file` is full path name
//
// Side Effects: Delete `file`
void Delete_File(char* file);

// Copy the file from `from` address to `to` address. `from` and `to` are
// full path names. Also copy the file permissions.
//
// Side Effets: copied `from` to `to`
void Copy_From_To(char* from, char* to);

// Recursively earch through dir_a_name and store files (paths relative to
// dir_a_name) in dir_a_files. Store number of files in dir_a_files in
// dir_a_num_files.
//
// Side Effects: modify char** dir_a_files, modify int dir_a_num_files
int get_dir_a_files(const char *filepath, const struct stat *info, 
		const int typeflag, struct FTW *pathinfo);


// Recursively earch through dir_b_name and store files (paths relative to
// dir_b_name) in dir_b_files. Store number of files in dir_b_files in
// dir_b_num_files.
//
// Side Effects: modify char** dir_b_files, modify int dir_b_num_files
int get_dir_b_files(const char *filepath, const struct stat *info, 
		const int typeflag, struct FTW *pathinfo);


// Global variables
// dir_a_files = filename relative to dir_a_name
// dir_b_files = filename relative to dir_b_name
char* dir_a_files [MAX_NUM_FILES];
int dir_a_num_files = 0; // number of files in dir_a
char* dir_b_files [MAX_NUM_FILES];
int dir_b_num_files = 0; // number of files in dir_b

// directory names
char* dir_a_name; 
char* dir_b_name;


// Main function
int main(int argc, char** argv)
{

	// Error check
	if(argc != 3)
	{
		fprintf(stderr, "Invalid Number of Arguments\n");
		exit(1);
	}

	// Get directory names, check to make sure that they are directories
	dir_a_name = argv[1]; dir_b_name = argv[2];
	if( dir_a_name[strlen(dir_a_name) - 1] != '/' ||
	 	 dir_b_name[strlen(dir_b_name) - 1] != '/' )
	{
		fprintf(stderr, "Directory Not Passed\n");
		exit(1);
	}


	// Get the files in dir_a_name and dir_b_name
	// Store files (paths relative to dir name) in dir_a_files and dir_b_files
	// dir_a_num_files and dir_b_num_files = num files in dir_a_name, dir_b_name
	// nftw recursively searches through directory
	if( nftw(dir_a_name, get_dir_a_files, USE_FDS, FTW_PHYS) < 0 )
		fprintf(stderr, "Error Using nftw on Directory %s\n", dir_a_name);
	if( nftw(dir_b_name, get_dir_b_files, USE_FDS, FTW_PHYS) < 0 )
		fprintf(stderr, "Error Using nftw on Directory %s\n", dir_b_name);



	// Compare files in directory `a` and directory `b` to see how to sync files
	// Make an array to keep track
	//		-1 = delete the file
	//		+1 = copy the file into directory `a`
	//		+2 = copy the file into directory `b`
	//		 0 = do nothing

	int DELETE_FILE = -1;
	int CP_TO_A = 1;
	int CP_TO_B = 2;
	
	int dir_a_sync[dir_a_num_files];
	memset(dir_a_sync, 0, dir_a_num_files * sizeof(int));
	int dir_b_sync[dir_b_num_files];
	memset(dir_b_sync, 0, dir_b_num_files * sizeof(int));
	
	// If file in `a` doesn't exist in `b`, copy the file in `a` to `b`
	for(int i = 0; i < dir_a_num_files; i++)
	{
		if( ! File_Exists(dir_a_files[i], dir_b_files, dir_b_num_files) )
		{
			// We will need to copy the file in `a` to `b`
			dir_a_sync[i] = CP_TO_B;
		}
	}
	
	// If file in `b` doesn't exist in `a`, delete the file in `b`
	for(int i = 0; i < dir_b_num_files; i++)
	{
		if( ! File_Exists(dir_b_files[i], dir_a_files, dir_a_num_files) )
		{
			// We will need to delete the file from `b`
			dir_b_sync[i] = DELETE_FILE;
		}
	}	
	
	// If the file in `a` exists in `b`, and file in `a` was modified 
	// more recently, copy from `a` to `b`
	for(int i = 0; i < dir_a_num_files; i++)
	{
		if( File_Exists(dir_a_files[i], dir_b_files, dir_b_num_files) )
		{
			// We will need to see which file was modified more recently
			struct stat file_in_a;
			char* file_in_a_path = Full_Path(dir_a_name, dir_a_files[i]);
			if(stat(file_in_a_path, &file_in_a) < 0)
				fprintf(stderr, "Error Accessing %s\n", file_in_a_path);

			struct stat file_in_b;
			char* file_in_b_path = Full_Path(dir_b_name, dir_a_files[i]);
			if(stat(file_in_b_path, &file_in_b) < 0)
				fprintf(stderr, "Error Accessing %s\n", file_in_b_path);
			
			// If `file_in_a` was modified more recently than `file_in_b`
			// Copy the file in `a` to `b`
			if( difftime(file_in_a.st_mtime, file_in_b.st_mtime) > 0 )
			{
				dir_a_sync[i] = CP_TO_B; // copy file in `a` to `b`
			}
			free(file_in_a_path); free(file_in_b_path);
		}
	}

	// If the file in `b` exists in `a` and the file in `b` was modified
	// more recently, copy from `b` to `a`
	for(int i = 0; i < dir_b_num_files; i++)
	{
		if( File_Exists(dir_b_files[i], dir_a_files, dir_a_num_files) )
		{
			// We will need to see which file was modified more recently
			struct stat file_in_a;
			char* file_in_a_path = Full_Path(dir_a_name, dir_a_files[i]);
			if(stat(file_in_a_path, &file_in_a) < 0)
				fprintf(stderr, "Error Accessing %s\n", file_in_a_path);

			struct stat file_in_b;
			char* file_in_b_path = Full_Path(dir_b_name, dir_a_files[i]);
			if(stat(file_in_b_path, &file_in_b) < 0)
				fprintf(stderr, "Error Accessing %s\n", file_in_b_path);

			// If `file_in_b` was modified more recently than `file_in_a`
			// Copy the file in `b` to `a`
			if( difftime(file_in_a.st_mtime, file_in_b.st_mtime) < 0 )
			{
				dir_b_sync[i] = CP_TO_A; // copy file in `b` to `a`
			}
			free(file_in_a_path); free(file_in_b_path);
		}
	}



	// Now do the specified operations
	
	// Look at all the files in a
	for(int i = 0; i < dir_a_num_files; i++)
	{
		// Delete the file from `a`
		if(dir_a_sync[i] == DELETE_FILE)
		{
			char* full_path = Full_Path(dir_a_name, dir_a_files[i]);
			Delete_File(full_path);
			fprintf(stdout, "Deleted %s\n", full_path);
			free(full_path);
		}
		
		// Copy the file from `a` to `b`
		else if(dir_a_sync[i] == CP_TO_B)
		{
			char* cp_from = Full_Path(dir_a_name, dir_a_files[i]);
			char* cp_to = Full_Path(dir_b_name, dir_a_files[i]);
			Copy_From_To(cp_from, cp_to);
			fprintf(stdout, "Copied %s to %s\n", cp_from, cp_to);
			free(cp_from); free(cp_to);			
		}
	}
	
	// Look at all the files in b
	for(int i = 0; i < dir_b_num_files; i++)
	{
		// Delete the file from `b`
		if(dir_b_sync[i] == DELETE_FILE)
		{
			char* full_path = Full_Path(dir_b_name, dir_b_files[i]);
			Delete_File(full_path);
			fprintf(stdout, "Deleted %s\n", full_path);
			free(full_path);			
		}
		
		// Copy the file from `b` to `a`
		else if(dir_b_sync[i] == CP_TO_A)
		{
			char* cp_from = Full_Path(dir_b_name, dir_b_files[i]);
			char* cp_to = Full_Path(dir_a_name, dir_b_files[i]);
			Copy_From_To(cp_from, cp_to);
			fprintf(stdout, "Copied %s to %s\n", cp_from, cp_to);
			free(cp_from); free(cp_to);			
		}
	}	
	
	exit(0);
}

// Create full path from directory name and file name. Returns dynamically
// allocated C string.
char* Full_Path(char* dir, char* file)
{
	char* out = malloc( strlen(dir) + strlen(file) + 1 );
	strcpy(out, dir); 
	strcat(out, file);
	return out;
}

// Delete file specified 
void Delete_File(char* file)
{
	if(remove(file) != 0)
	{
		fprintf(stderr, "Unable to Delete File%s\n", file);
	}
}


// Copy file `from` to `to`. Copy file contents and file permissions
// Arguments need to be the full path of the file.
void Copy_From_To(char* from, char* to)
{
	// Get file permissions of `from` file
	// Copy these file permissions
	struct stat from_file_permissions; stat(from, &from_file_permissions);

	// Open files, check for errors
	FILE* from_file = fopen(from, "r");
	FILE* to_file = fopen(to, "w+");

// Need to mkdir
//	if (!is_dir($myDir)) {
//		mkdir($myDir, 0777, true); // true for recursive create
//	}

	if(from_file == NULL)
		fprintf(stderr, "Error Opening %s\n", from);

	if(to_file == NULL)
	{
		fprintf(stderr, "Error Opening %s\n", to);
	}

	char buffer[256];
	int bytes_read;

	while(1)
	{
		size_t bytes_read = fread(buffer, sizeof(char), sizeof(buffer), from_file);
		fwrite(buffer, sizeof(char), bytes_read, to_file);
		if(bytes_read < sizeof(buffer) && feof(from_file))
			break;
	}
	fclose(from_file); fclose(to_file); // close FILEs

	// copy file permissions	
	if( chmod(to, from_file_permissions.st_mode & 07777) == -1 )
		fprintf(stderr, "Error Changing File Permissions for %s\n", to);
}

// Does the `file` exist in `array`?
bool File_Exists(char* file, char** array, int array_size)
{
	for(int i = 0; i < array_size; i++)
	{
		if(strcmp(file, array[i]) == 0)
			return true;
	}
	return false;
}


// nftw runs this function. Recursively look through dir_a_name and save
// files relative to dir_a_name directory in dir_a_files.
int get_dir_a_files(const char *filepath, const struct stat *info, 
		const int typeflag, struct FTW *pathinfo)
{
	// Regular file?
	if (typeflag == FTW_F) 
	{
		int offset = strlen(dir_a_name);
		dir_a_files[dir_a_num_files] = malloc( strlen(filepath) + 1 - offset );
		strcpy(dir_a_files[dir_a_num_files], &filepath[offset]);
		dir_a_num_files++; // keep track of number of files
	}
	return 0;
}


// nftw runs this function. Recursively look through dir_b_name and save
// files relative to dir_b_name directory in dir_b_files.
int get_dir_b_files(const char *filepath, const struct stat *info, 
		const int typeflag, struct FTW *pathinfo)
{
	// Regular file?
	if (typeflag == FTW_F) 
	{
		int offset = strlen(dir_b_name);
		dir_b_files[dir_b_num_files] = malloc( strlen(filepath) + 1 - offset );
		strcpy(dir_b_files[dir_b_num_files], &filepath[offset]);
		dir_b_num_files++; // keep track of number of files
	}
	return 0;
}

