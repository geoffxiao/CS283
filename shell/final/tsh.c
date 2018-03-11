/*
* CS283 Shell Project
*
* tsh - A tiny shell program with job control
*
* Geoffrey Xiao
* gx26
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "csapp.h"

/* Misc manifest constants */
#define MAXARGS 128 /* max args on a command line */
#define MAXJOBS 16 /* max jobs at any point in time */
#define MAXJID 1<<16 /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1 /* running in foreground */
#define BG 2 /* running in background */
#define ST 3 /* stopped */

/*
* Jobs states: FG (foreground), BG (background), ST (stopped)
* Job state transitions and enabling actions:
*     FG -> ST  : ctrl-z
*     ST -> FG  : fg command
*     ST -> BG  : bg command
*     BG -> FG  : fg command
* At most 1 job can be in the FG state.
*/

/* Global variables */
extern char **environ; /* defined in libc */
char prompt[] = "tsh> "; /* command line prompt (DO NOT CHANGE) */
int verbose = 0; /* if true, print additional output */
int nextjid = 1; /* next job ID to allocate */
char sbuf[MAXLINE]; /* for composing sprintf messages */

struct job_t
{ 
	/* The job struct */
	pid_t pid; /* job PID */
	int jid; /* job ID [1, 2, ...] */
	int state; /* UNDEF, BG, FG, or ST */
	char cmdline[MAXLINE]; /* command line */ 
};

struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */

/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv);
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs);
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid);
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid);
int pid2jid(pid_t pid);
void listjobs(struct job_t *jobs);

void usage(void);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);


// --------------------------------------------------------------------------//
/* My own helper functions */

// wrapper for pipe function
int pipe_func( int fd[2] );

// Does array contain str?
int contains_string(char** array, char* str);

// Redirect stdin/stdout and run command
void redirect_and_run(char** argv1, char** argv2, 
							 char* redirect_type, int open_flag, int old_fd);

// clean up heap allocated variables
void cleanup_heap(char** argv1, char** argv2);

// SIGCHLD handler for running pipes
void pipe_sigchld_handler(int sig);

// main shell process when running jobs
void main_shell_process(pid_t pid, char* job_str, int bg, sigset_t mask);

// parse arguments
void parse_argv1_argv2(char** argv1, char** argv2, char** argv, int loc);
// --------------------------------------------------------------------------//

/* End function headers */



/* My helper functions */

// --------------------------------------------------------------------------//
// wrapper for pipe system call
int pipe_func( int fd[2] )
{
	if( pipe(fd) < 0 )
	{
		printf("tsh: Error Using pipe in tsh.c\n");
		return -1;
	}
	else
	{
		return 1;
	}
}

// Does array contain str?
// null terminated array
// return -1 if not in array
// else return index of the str
int contains_string(char** array, char* str)
{
	char* tmp;
	for(int i = 0; (tmp = array[i]) != NULL; i++)
	{
		if(strcmp(str, tmp) == 0)
			return i;
	}
	return -1;
}

// Redirect stdin/stdout and run command
void redirect_and_run(char** argv1, char** argv2, 
							 char* redirect_type, int open_flag, int old_fd)
{
	int redirect_fd = Open(argv2[0], open_flag,
								  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	// create new file, default U=RW,G=RW,O=R

	// redirection
	if(verbose)
		printf("tsh: Run %s and Redirect %s with %s\n",
 	   		argv1[0], redirect_type, argv2[0]);
   Dup2(redirect_fd, old_fd); // old_fd -> redirect
	Execve(argv1[0], argv1, environ); // load and run program 
	exit(1); // should never reach unless execve error
}

// clean up null-terminated arrays argv1 and argv2
void cleanup_heap(char** argv1, char** argv2)
{
	// free heap
	int i = 0;
	char* tmp_ptr = argv1[i];
	while(tmp_ptr != NULL)
	{
		free(argv1[i]);
		i++;
		tmp_ptr = argv1[i];
	}

	i = 0; tmp_ptr = argv2[i];
	while(tmp_ptr != NULL)
	{
		free(argv2[i]);
		i++;
		tmp_ptr = argv2[i];
	}
}

// Add job PID which is runing command job_str
// bg = run in background or foreground
// mask = signal mask
void main_shell_process(pid_t pid, char* job_str, int bg, sigset_t mask)
{
	if( !bg ) // FG job 
	{
		addjob(jobs, pid, FG, job_str); // add job
		sigprocmask(SIG_UNBLOCK, &mask, NULL); // unblock signals
		waitfg(pid); // Allow job to run in FG
	}
	else // BG job
	{
		addjob(jobs, pid, BG, job_str); // add job
		sigprocmask(SIG_UNBLOCK, &mask, NULL); // unblock signals

		int jid = pid2jid(pid);
		printf("tsh: Job [%d] (%d) Running in Background\n", 
				jid, pid);
	}
}

// argv = full commandline parsed into array
// argv1 = first part of commandline
// argv2 = second part of commandline
// If commandline is seperated by pipe (|) or redirect (>, <, >>)
void parse_argv1_argv2(char** argv1, char** argv2, char** argv, int loc)
{
	// copy into argv1
	int c = 0;
	while(c < loc)
	{
		argv1[c] = malloc(strlen(argv[c]) + 1);
		strcpy(argv1[c], argv[c]);
		c++;
	}
	argv1[c] = NULL;

	// copy into argv2
	argv2[0] = malloc(strlen(argv[c + 1]) + 1);
	strcpy(argv2[0], argv[c + 1]);
	argv2[1] = NULL;

	return;
}
// --------------------------------------------------------------------------//

/* End my helper functions */


/*
* main - The shell's main routine
*/
int main(int argc, char **argv)
{
	char c;
	char cmdline[MAXLINE];
	int emit_prompt = 1;
	/* emit prompt (default) */

	/* Redirect stderr to stdout (so that driver will get all output
	* on the pipe connected to stdout) */
	Dup2(1, 2);

	/* Parse the command line */
	while ((c = getopt(argc, argv, "hvp")) != EOF)
	{
		switch (c)
		{
			case 'h': /* print help message */
				usage();
				break;
      
			case 'v': /* emit additional diagnostic info */
				verbose = 1;
				break;
      
			case 'p': /* don't print a prompt */
				emit_prompt = 0;
				/* handy for automatic testing */
				break;
      
			default:
				usage();
		}
	}

  
	/* Install the signal handlers */

	/* These are the ones you will need to implement */
	Signal(SIGINT, sigint_handler); /* ctrl-c */
	Signal(SIGTSTP, sigtstp_handler); /* ctrl-z */
	Signal(SIGCHLD, sigchld_handler); /* Terminated or stopped child */

	/* This one provides a clean way to kill the shell */
	Signal(SIGQUIT, sigquit_handler);

	/* Initialize the job list */
	initjobs(jobs);

	/* Execute the shell's read/eval loop */
  	while (1)
	{

		/* Read command line */    
		if (emit_prompt)
		{
			printf("%s", prompt);
			fflush(stdout);
		}
    
		if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
			app_error("fgets error");
    
		if (feof(stdin))
		{
			/* End of file (ctrl-d) */
			fflush(stdout);
			exit(0);
		}
			
		/* evaluate the command line */
		eval(cmdline); 
		fflush(stdout);
		fflush(stdout);
	}

	exit(0);
	/* control never reaches here */
}


/*
* eval - Evaluate the command line that the user has just typed in
*
* If the user has requested a built-in command (quit, jobs, bg or fg)
* then execute it immediately. Otherwise, fork a child process and
* run the job in the context of the child. If the job is running in
* the foreground, wait for it to terminate and then return.  Note:
* each child process must have a unique process group ID so that our
* background children don't receive SIGINT (SIGTSTP) from the kernel
* when we type ctrl-c (ctrl-z) at the keyboard.
*/

void eval(char *cmdline)
{
	char * argv[256];
	char * argv1[256];
	char * argv2[256];

	argv1[0] = NULL; argv2[0] = NULL; // initialize

	char buf[256];

	// for file redirection
	int open_flag; int old_fd;
	char* redirect_type;
	
	int redirect_flag = -1; // -1, no redirect
	int pipe_flag = -1; // -1, no pipe

	// parse commandline
	strcpy(buf, cmdline);
	int bg = parseline(buf, argv);


	// parse the cmdline for redirects
	int loc;

	// STDOUT redirection
	if((loc = contains_string(argv, ">>")) > 0)
	{
	   // parse the ">>" redirect
		parse_argv1_argv2(argv1, argv2, argv, loc);

		// stuff needed for redirect		
		open_flag = O_APPEND | O_WRONLY;
		old_fd = STDOUT_FILENO;
		
		redirect_flag = 1;
		redirect_type = "STDOUT (Append)";
	}
	// STDOUT redirection
	else if((loc = contains_string(argv, ">")) > 0)
	{
	   // parse the ">" redirect
		parse_argv1_argv2(argv1, argv2, argv, loc);

		// stuff needed for redirect		
		open_flag = O_CREAT | O_TRUNC | O_WRONLY;
		old_fd = STDOUT_FILENO;
		
		redirect_flag = 1;
		redirect_type = "STDOUT";
	}
	// STDIN redirection
	else if((loc = contains_string(argv, "<")) > 0)
	{
	   // parse the "<" redirect
		parse_argv1_argv2(argv1, argv2, argv, loc);
		
		// stuff needed for pipe/redirect		
		open_flag = O_RDONLY;
		old_fd = STDIN_FILENO;
		
		redirect_flag = 1;
		redirect_type = "STDIN";
	}
	// No pipe support for builtin commands
	// pipe
	else if((loc = contains_string(argv, "|")) > 0)
	{
	   // parse the "|" redirect
		// copy into argv1
		int c = 0;
		while(c < loc)
		{
			argv1[c] = malloc(strlen(argv[c]) + 1);
			strcpy(argv1[c], argv[c]);
			c++;
		}
		argv1[c] = NULL;

		// copy into argv2
	  	int start = loc + 1;
		c = start; // loc + 1 b/c we want to skip the ">"
		char* tmp_ptr = argv[start];
		while( tmp_ptr != NULL )
		{
			argv2[c - start] = malloc(strlen(argv[c]) + 1);
			strcpy(argv2[c - start], argv[c]);
			c++; tmp_ptr = argv[c];
		}
		argv2[c - start] = NULL;

		// for pipe
		pipe_flag = 1;
	}
	// No redirection + No pipes
	else
	{
		// copy into argv1
		int c = 0;
		char* tmp_ptr = argv[c];
		while( tmp_ptr != NULL )
		{
			argv1[c] = malloc(strlen(argv[c]) + 1);
			strcpy(argv1[c], argv[c]);
			c++; tmp_ptr = argv[c];
		}
		argv1[c] = NULL;
	}

	if( argv[0] == NULL ) // empty line
	{
		return;
	}

	int builtin = builtin_cmd(argv1); // run builtin

	if(!builtin) // not a builtin
	{
		// Prevents race harzard between adding a job and the job ending
		sigset_t mask; // mask for signal blocking in the main shell
		Sigemptyset(&mask);
		Sigaddset(&mask, SIGCHLD);
		Sigaddset(&mask, SIGINT);
		Sigaddset(&mask, SIGTSTP);
		Sigprocmask(SIG_BLOCK, &mask, NULL);

		if( redirect_flag > 0 ) // redirection
		{
			int pid = Fork();
			if( pid == 0 ) // child
			{
				if( argv2[0] == NULL )
				{
					printf("tsh: No Redirection File Provided\n");
					exit(1);
				}

				if( access( argv1[0], F_OK ) == -1 ) 
				{
					printf("tsh: Error %s Does not Exist\n", argv1[0]);
					exit(1);
				}

				// distinguish main shell process group pid from job process group pid
				Sigprocmask(SIG_UNBLOCK, &mask, NULL); // unblock signals
				Setpgid(0, 0); // allow children of this child to have new pid
				redirect_and_run(argv1, argv2, redirect_type, open_flag, old_fd); 
				exit(1); // should reach only if execve messed up
			}
			else // pid > 0, parent
			{
				main_shell_process(pid, argv1[0], bg, mask);
			}
		}
		else if( pipe_flag > 0 ) // pipe
		{
			// Create file descriptors for pipe
			int pipe_fd[2];
			if( pipe_func(pipe_fd) < 0 )
			{
				printf("tsh: Error Using pipe in tsh.c (%s)\n",
						strerror(errno));
				cleanup_heap(argv1, argv2);
				return;
			}
				
			int pid1; int pid2; // create two children
			pid1 = Fork();
			if( pid1 == 0 ) // child1 runs program1
			{
				// Does the program exist?
				if( access( argv1[0], F_OK ) == -1 ) 
				{
					printf("tsh: Error %s Does not Exist\n", argv1[0]);
					exit(1);
				}

				Sigprocmask(SIG_UNBLOCK, &mask, NULL); // unblock signals	
				Setpgid(0, 0); // distinguish job processes from main shell

				Close(pipe_fd[0]);  // don't need
				Dup2(pipe_fd[1], STDOUT_FILENO); // program1 STDOUT -> pipe_fd[1] (for writing)
						
				Execve(argv1[0], argv1, environ); // load and run program
				exit(1);
			}

			pid2 = Fork();
			if( pid2 == 0 ) // child2 runs program2
			{ 
				// Does the program exist?
				if( access( argv2[0], F_OK ) == -1 ) 
				{
					printf("tsh: Error %s Does not Exist\n", argv2[0]);
					exit(1);
				}

				Sigprocmask(SIG_UNBLOCK, &mask, NULL); // unblock signals	
				Setpgid(0, pid1); // distinguish job processes from main shell

				Close(pipe_fd[1]); // don't need
				Dup2(pipe_fd[0], STDIN_FILENO); // program2 STDIN -> pipe_fd[0] (for reading)
			
				Execve(argv2[0], argv2, environ); // load and run program
				exit(1);
			}
			
	
			// main shell	
			if(verbose)
				printf("tsh: Run %s | %s\n", argv1[0], argv2[0]);

			char* job_str = malloc(strlen(argv1[0]) + strlen(argv2[0]) +
					strlen(" | ") + 1);
			strcpy(job_str, argv1[0]);
			strcat(job_str, " | ");
			strcat(job_str, argv2[0]);		
	
			main_shell_process(pid1, job_str, bg, mask);
			free(job_str);
		}
		else // no pipe or redirect
		{
			int pid = Fork();
			if( pid == 0 ) // child
			{
				// Does the argv1[0] program exist?
				if( access( argv1[0], F_OK ) == -1 ) 
				{
					printf("tsh: Error %s Does not Exist\n", argv1[0]);
					exit(1);
				}

				// distinguish main shell process group pid from job process group pid
				Sigprocmask(SIG_UNBLOCK, &mask, NULL); // unblock signals
				Setpgid(0, 0); // allow children of this child to have new pid
				Execve(argv1[0], argv1, environ); // load and run program	
				exit(1); // should only reach if execve messed up
			}
			else // pid > 0, parent
			{
				main_shell_process(pid, argv1[0], bg, mask);			
			}
		}
	}
	
	cleanup_heap(argv1, argv2);
	return;
}


/*
* parseline - Parse the command line and build the argv array.
*
* Characters enclosed in single quotes are treated as a single
* argument.  Return true if the user has requested a BG job, false if
* the user has requested a FG job.
*/
int parseline(const char *cmdline, char **argv)
{
	static char array[MAXLINE]; /* holds local copy of command line */
	char *buf = array; /* ptr that traverses command line */
	char *delim; /* points to first space delimiter */
	int argc; /* number of args */
	int bg; /* background job? */
  
	strcpy(buf, cmdline);
	buf[strlen(buf)-1] = ' '; /* replace trailing '\n' with space */
	while (*buf && (*buf == ' ')) /* ignore leading spaces */
		buf++;

  
	/* Build the argv list */
	argc = 0;
	if (*buf == '\'')
  	{
	  	buf++;
		delim = strchr(buf, '\'');
	}
 	else
	{
  		delim = strchr(buf, ' ');
 	}

 	while (delim)
	{
  		argv[argc++] = buf;
		*delim = '\0';
	 	buf = delim + 1;
	  	while (*buf && (*buf == ' ')) /* ignore spaces */
			buf++;

		if (*buf == '\'')
		{
		  	buf++;
		  	delim = strchr(buf, '\'');
		}
		else
		{
		  	delim = strchr(buf, ' ');
		}
	}
	argv[argc] = NULL;
  
	if (argc == 0) /* ignore blank line */
		return 1;
  
	/* should the job run in the background? */
	if ((bg = (*argv[argc-1] == '&')) != 0)
	{
		argv[--argc] = NULL;
	}
	return bg; 
}

/*
* builtin_cmd - If the user has typed a built-in command then execute
*    it immediately.
*
* return 1 if builtin command
* Supported builtins = quit, jobs, bg, fg
*/
int builtin_cmd(char **argv)
{
	char* cmd = argv[0];

	if( !strcmp(cmd, "quit") )
	{
		exit(0); // quit command
	}
	else if( !strcmp(cmd, "&") )
	{
		return 0; // ignore singleton & command
	}
	else if( !strcmp(cmd, "jobs") )
	{
		listjobs(jobs); // print jobs
		return 1;
	}
	else if( !strcmp(cmd, "bg") || !strcmp(cmd, "fg") )
	{
		do_bgfg(argv);
		return 1;
	}
	else
	{
		return 0;
	}
	/* not a builtin command */
}


/*
* do_bgfg - Execute the builtin bg and fg commands
*/
void do_bgfg(char **argv)
{
	if( argv[1] == NULL ) // no second argument in fg or bg
	{
		printf("tsh: %s Needs an Argument\n", argv[0]);
		return;
	}

	// the argument after bg or fg
	char* bgfg_arg = malloc( strlen(argv[1]) + 1 ); // Extract arg after 'bg' or 'fg'
	strcpy(bgfg_arg, argv[1]);

	int job_id; 
	struct job_t * job; 
	pid_t job_pid;

	if( bgfg_arg[0] == '%' ) // fg %#, referencing job ID
	{
		// which job to fg?
		job_id = atoi( &bgfg_arg[1] );
		job = getjobjid(jobs, job_id);
		if( job == NULL )
		{
			printf("tsh: Job ID %d Does Not Exist\n", job_id);
			return;
		}
		job_pid = job->pid;
	}
	else // fg #, # is the PID
	{
		// What job are we trying to operate on?
		job_pid = (pid_t) atoi( bgfg_arg ); // convert to pid
		job_id = pid2jid(job_pid);
		job = getjobjid(jobs, job_id);
		// The job we are trying to bg or fg does not exist
		if( job == NULL )
		{
				printf("tsh: PID %d Does Not Exist\n", job_pid);
			return;
		}
	}

	// fg, switch background/suspended job to foreground
	if( !strcmp(argv[0], "fg") )
	{
		// check job state if ST? or BG?
		if( job->state == ST || job->state == BG )
		{
			job->state = FG; // set state
			Kill(-job_pid, SIGCONT); // send SIGCONT
			waitfg(job_pid); // allow FG job to run
		}
		else // can't fg a job already in FG
		{
			printf("tsh: Job [%d] (%d) is Already in Foreground\n",
					job->jid, job_pid);
		}
	}

	// bg, restart a suspended job in background
	else if( !strcmp(argv[0], "bg") )
	{
		// check job state if ST?
		if( job->state == ST )
		{
			job->state = BG; // set state
			Kill(-job_pid, SIGCONT); // send SIGCONT
			printf("tsh: Running Job [%d] (%d) in Background\n", job->jid,
					job_pid);
		}
		else // can only bg a job that is ST
		{
			printf("tsh: Job [%d] (%d) is Not Stopped\n",
					job->jid, job_pid);
		}
	}

	free(bgfg_arg);
	return;
}

/*
* waitfg - Block until process pid is no longer the foreground process
*/

// called by the main shell
void waitfg(pid_t pid)
{
	struct job_t * fg_job = getjobpid(jobs, pid);
	
	// once FG job finishes, SIGCHLD sent to parent, main shell
	// Block SIGTERM, SIGINT signals to main shell, we want these signals to
	// go to FG job instead
	// To block these signals, simply put main shell to sleep

	// Keep sleeping while fg job is running
	while( fgpid(jobs) == pid )
	{
		Sleep(1); // put main shell to sleep	
	}

	if(verbose)
	{
		printf("tsh: Process (%d) No Longer the FG Process\n", pid);
	}
	return;
}

/*****************
	* Signal handlers
	*****************/

/*
* sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
*     a child job terminates (becomes a zombie), or stops because it
*     received a SIGSTOP or SIGTSTP signal. The handler reaps all
*     available zombie children, but doesn't wait for any other
*     currently running children to terminate.
*/

// A child (a tsch job) finished
void sigchld_handler(int sig)
{		
	if( verbose )
	{
		printf("tsh: sigchld_handler Entered by PID %d\n", getpid());
	}

	pid_t child_pid;
	int status;

	// reap zombie children of main tsh shell process
	// WNOHANG = don't wait for children running to terminate/stop
	// WUNTRACED = look for term/stopped chldren

	// Use while loop to reap as many children as possible
	while( (child_pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0 ) // See which tsch jobs term/stop
	{
		struct job_t * child_job = getjobpid(jobs, child_pid); // Get job id for pid 

		if( child_job == NULL )
			return;

		if( WIFEXITED(status) ) // tsh job exited normally
		{
			if(verbose)
			{
				printf("tsh: Job [%d] (%d) Exited Normally Signal %d\n", child_job->jid,
					child_job->pid, WTERMSIG(status));
				printf("tsh: Job [%d] (%d) Deleted\n", child_job->jid,
						child_job->pid);
			}
			deletejob(jobs, child_pid); // delete tsh job
		}
		else if( WIFSIGNALED(status) ) // tsh job term b/c unhandled error
		{
			printf("tsh: Job [%d] (%d) Terminated by Signal %d\n",
					child_job->jid, child_job->pid, WTERMSIG(status));
			if(verbose)
				printf("tsh: Job [%d] (%d) Deleted\n", child_job->jid,
						child_job->pid);
			deletejob(jobs, child_pid);
		}
		else if( WIFSTOPPED(status) ) // tsh job stopped
		{
			printf("tsh: Job [%d] (%d) Stopped by Signal %d\n", child_job->jid,
					child_job->pid, WSTOPSIG(status));
			child_job->state = ST; // update jobs array
			if(verbose)
				printf("tsh: Job [%d] (%d) State Set to ST\n", child_job->jid,
						child_job->pid);
		}	
	}

	if( verbose )
	{
		printf("tsh: sigchld_handler Exited by PID %d\n", getpid());
	}
	return;
}

/*
* sigint_handler - The kernel sends a SIGINT to the shell whenver the
*    user types ctrl-c at the keyboard.  Catch it and send it along
*    to the foreground job.
*/

// ctrl+c in tsh
// Need to terminate current fg job in tsh
void sigint_handler(int sig)
{
	pid_t fg_pid = fgpid(jobs); // current foreground job
	struct job_t * fg_job = getjobpid(jobs, fg_pid);

	if( verbose && fg_pid != 0 )
		printf("tsh: sigint_handler Entered by PID %d\n", getpid());

	if( fg_pid == 0 ) // No current jobs
	{
		printf("tsh: No Foreground Jobs\n");
		return;
	}
	
	Kill(-fg_pid, sig); // kill this job and all others in its process group
	if( verbose )
	{
		printf("tsh: Job [%d] (%d) Terminated by Ctrl+C\n", fg_job->jid, fg_pid);	
		printf("tsh: sigint_handler Exited by PID %d\n", getpid());
	}
}

/*
* sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
*     the user types ctrl-z at the keyboard. Catch it and suspend the
*     foreground job by sending it a SIGTSTP.
*/

// ctrl+z in tsch
// Need to suspend the current fg tsch job
void sigtstp_handler(int sig)
{	
	pid_t fg_pid = fgpid(jobs); // current foreground job
	struct job_t * fg_job = getjobpid(jobs, fg_pid);

	if( verbose && fg_pid != 0 )
		printf("tsh: sigtstp_handler Entered by PID %d\n", getpid());

	if( fg_pid == 0 )
	{
		printf("tsh: No Foreground Jobs\n");
		return;
	}
	
	Kill(-fg_pid, sig); // send signal to stop job
	if( verbose )
	{
		printf("tsh: Job [%d] (%d) Stopped by Ctrl+Z\n", fg_job->jid, fg_job->pid);
		printf("tsh: sigtstp_handler Exited by PID %d\n", getpid());
	}
}

/*********************
* End signal handlers
*********************/


//-------------------- Nothing changed below -----------------------//

/***********************************************
* Helper routines that manipulate the job list
**********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job)
{
  	job->pid = 0;
  	job->jid = 0;
  	job->state = UNDEF; 
  	job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs)
{
 	int i;
	for (i = 0; i < MAXJOBS; i++)
		clearjob(&jobs[i]); 
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs)
{
 	int i, max=0;
  	for (i = 0; i < MAXJOBS; i++)
	  	if (jobs[i].jid > max)
		  	max = jobs[i].jid;
	return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
 	int i;
 
  	// can't have job pid < 1
  	if (pid < 1)
	  	return 0;
  
	// search for next free space in jobs array
	for (i = 0; i < MAXJOBS; i++)
  	{
		if (jobs[i].pid == 0)
 		{
			jobs[i].pid = pid;
	  		jobs[i].state = state;
		 	jobs[i].jid = nextjid++;
			if (nextjid > MAXJOBS)
				nextjid = 1;
			strcpy(jobs[i].cmdline, cmdline);
	  		if(verbose)
			{
				printf("tsh: Added job [%d] %d %s\n",
					  	jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
			}
			return 1;
		}
	}
 	printf("tsh: Tried to create too many jobs\n");
 	return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid)
{
 	int i;
 	if (pid < 1)
	 	return 0;

	for (i = 0; i < MAXJOBS; i++)
  	{
		if (jobs[i].pid == pid)
		{
	  		clearjob(&jobs[i]);
		 	nextjid = maxjid(jobs)+1;
			return 1;
		}
	}
	return 0; 
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs)
{
 	int i;
	for (i = 0; i < MAXJOBS; i++)
		if (jobs[i].state == FG)
			return jobs[i].pid;
	
	return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid)
 
{
 	int i;

  	if (pid < 1)
	  	return NULL;
  	
	for (i = 0; i < MAXJOBS; i++)
	  	if (jobs[i].pid == pid)
			return &jobs[i];
 	
	return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid)
 {
  int i;

  if (jid < 1)
  return NULL;
  for (i = 0; i < MAXJOBS; i++)
  if (jobs[i].jid == jid)
  return &jobs[i];
  return NULL;
 }

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid)
 
{
	int i;

	if (pid < 1)
		return 0;
 
  	for (i = 0; i < MAXJOBS; i++)
	  	if (jobs[i].pid == pid)
		{
			return jobs[i].jid;
		}
	
	return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs)
{
	int i;

	for (i = 0; i < MAXJOBS; i++)
	{
	  	if (jobs[i].pid != 0)
		{
		  	printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
      
			switch (jobs[i].state)
			{
				case BG:
					printf("Running ");
					break;
      
			 	case FG:
			 		printf("Foreground ");
					break;
 
	  			case ST:
			  		printf("Stopped ");
				  	break;
   
		 		default:
				 	printf("listjobs: Internal error: job[%d].state=%d",
						  	i, jobs[i].state);
			}
			printf("%s\n", jobs[i].cmdline);
		}
	}
}


/******************************
* end job list helper routines
******************************/


/***********************
* Other helper routines
***********************/

/*
* usage - print a help message
*/
void usage(void)
{
  	printf("Usage: shell [-hvp]\n");
 	printf(" -h print this message\n");
 	printf(" -v print additional diagnostic information\n");
 	printf(" -p do not emit a command prompt\n");
 	exit(1); 
}

/*
* sigquit_handler - The driver program can gracefully terminate the
*    child shell by sending it a SIGQUIT signal.
*/
void sigquit_handler(int sig)
{
	 printf("Terminating after receipt of SIGQUIT signal\n");
	 exit(1);
}
