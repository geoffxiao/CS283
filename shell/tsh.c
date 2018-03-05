/*
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

/* Misc manifest constants */
#define MAXLINE 1024 /* max line size */
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
 { /* The job struct */
  pid_t pid; /* job PID */
  int jid; /* job ID [1, 2, ...] */
  int state; /* UNDEF, BG, FG, or ST */
  char cmdline[MAXLINE]; /* command line */
 }
;
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
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);


// Error wrapper for kill
// Return 0 and print error message if kill failed
// Return 1 if OK
int Kill(pid_t pid, int sig)
{
	if( kill(pid, sig) < 0)
	{
		printf("Error Using kill in tsh.c\n");
		return 0;
	}
	return 1;
}


// first_arg...whitespace...parse...whitespace...second_arg...whitespace
void parse_pipe_redirect(char* cmdline, char** first_arg, char** second_arg, char parse)
{
    char c;
    int len_first_arg = 0;

    // find where 'parse' begins
    while( (c = cmdline[len_first_arg]) != parse )
    {
        len_first_arg++;
    }
    
    int start_second_arg = len_first_arg + 1; // start of second_arg
    len_first_arg--; // backtrack before the parse
    
    // remove whitespace    
    while( (c = cmdline[len_first_arg]) == ' ' )
    {
        len_first_arg--;
    }
    len_first_arg = len_first_arg + 1;
    
    // populate first_arg
    first_arg[0] = malloc(len_first_arg + 1);
    for(int i = 0; i < len_first_arg; i++)
    {
        first_arg[0][i] = cmdline[i];
    }
    first_arg[0][len_first_arg] = '\0';
 
    // second_arg
    // remove whitespace
    while( (c = cmdline[start_second_arg]) == ' ' )
    {
        start_second_arg++;
    }    
    
    // populate second_arg
    second_arg[0] = malloc(strlen(cmdline) - start_second_arg);
    for(int i = start_second_arg; i < strlen(cmdline) - 1; i++)
    {
        second_arg[0][i - start_second_arg] = cmdline[i];
    }
    
}



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
  dup2(1, 2);

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

	 if( strstr(cmdline, "|") ) // pipe
	 {
		// second_cmd stdin is first_cmd stdout
		
	 }
	 else if( strstr(cmdline, ">") ) // file redirection
	 {   
	 	char* first_cmd[1]; char* second_cmd[1];
		parse_pipe_redirect(cmdline, first_cmd, second_cmd, '>');

		printf("%s\n", second_cmd[0]);

		int redirect = open(second_cmd[0], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR); // create new file 
		int old_stdout = dup(STDOUT_FILENO); // old_stdout -> STDOUT

		dup2(redirect, STDOUT_FILENO); // STDOUT -> redirect


//	STRING PROCESSING FIX
		// Add a " " (space) to first_cmd so we an parse correctly in eval
		char* first_cmd2 = malloc(strlen(first_cmd[0]) + 2);
		strcpy(first_cmd2, first_cmd[0]);
		strcat(first_cmd2, " ");

		eval(first_cmd2); // eval command using STDOUT -> redirect
		dup2(old_stdout, STDOUT_FILENO); // STDOUT -> old_stdout

		// close fds
		close(redirect); close(old_stdout);

		// free heap
		free(first_cmd2); free(first_cmd[0]); free(second_cmd[0]);
	 }
	 else if( strstr(cmdline, "<") ) // file redirection
	 {
	    	
	 }
	 else
	 {
	 	/* Evaluate the command line */
    	eval(cmdline);
	 }
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
	char buf[256];

	strcpy(buf, cmdline);
	int bg = parseline(buf, argv); // bg = 1 if to run job in bg

	sigset_t mask; // mask for signal blocking in the main shell

	if( argv[0] == NULL ) // empty line
		return;

	if( !builtin_cmd(argv) ) // try to run builtin_cmd
	{
		sigemptyset(&mask);
		sigaddset(&mask, SIGCHLD);
		sigaddset(&mask, SIGINT);
		sigaddset(&mask, SIGTSTP);
		sigprocmask(SIG_BLOCK, &mask, NULL);

		// Does the argv[0] program exist?
		if( access( argv[0], F_OK ) == -1 ) 
		{
			printf("Error %s Does not Exist\n", argv[0]);
			return;
		}

		int pid;
		if( (pid = fork()) < 0 )
		{
			printf("Error using fork in tsh.c\n");
		}

		if( pid == 0 ) // child, load and run program using execve
		{
			// distinguish main shell process group pid from job process group pid
			sigprocmask(SIG_UNBLOCK, &mask, NULL); // unblock signals
			setpgid(0, 0); // allow children of this child to have new pid
			execve(argv[0], argv, environ); // load and run program
		}
		else if( pid > 0 )// parent, main shell
		{
			if( !bg ) // FG job 
			{
				addjob(jobs, pid, FG, argv[0]);
				sigprocmask(SIG_UNBLOCK, &mask, NULL); // unblock signals
				waitfg(pid); // Allow job to run in FG
			}
			else // BG job
			{
				addjob(jobs, pid, BG, argv[0]);
				sigprocmask(SIG_UNBLOCK, &mask, NULL); // unblock signals

				// FIX
				printf("Job [%d] (%d) Running in Background\n", pid, pid); // FIX!!!
			}
		}
	}
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
		return 1; // ignore singleton & command
	}
	else if( !strcmp(cmd, "jobs") )
	{
		listjobs(jobs); // print jobs
		printf("\n");
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
		printf("%s Needs an Argument\n", argv[0]);
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
			printf("Job ID %d Does Not Exist\n", job_id);
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
			printf("PID %d Does Not Exist\n", job_pid);
			return;
		}
	}

	// fg, switch background/suspended job to foreground
	if( !strcmp(argv[0], "fg") )
	{
		// check job state if ST? or BG?
		if( job->state == ST || job->state == BG )
		{
			// send SIGCONT to job_pid
			if( Kill(-job_pid, SIGCONT) ) 
			{
				job->state = FG;
				waitfg(job_pid); // allow FG job to run
			}
		}
		else // can't fg a job already in FG
		{
			printf("Job [%d] (%d) is Already in Foreground\n",
					job->jid, job_pid);
		}
	}

	// bg, restart a suspended job in background
	else if( !strcmp(argv[0], "bg") )
	{
		// check job state if ST?
		if( job->state == ST )
		{
			// send SIGCONT to job_pid
			if( Kill(-job_pid, SIGCONT) ) 
			{
				job->state = BG;
			}
		}
		else // can only bg a job that is ST
		{
			printf("Job [%d] (%d) is Not Stopped\n",
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
		sleep(1); // put main shell to sleep	
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
	pid_t child_pid;
	int status;

	// reap zombie children of main tsh shell process
	// WNOHANG = don't wait for children running to terminate/stop
	// WUNTRACED = look for term/stopped chldren

	// Use while loop to reap as many children as possible
	while( (child_pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0 ) // See which tsch jobs term/stop
	{
		struct job_t * child_job = getjobpid(jobs, child_pid); // Get job id for pid 

		if( verbose )
		{
			printf("sigchld_handler Entered by PID %d\n", getpid());
		}

		if( WIFEXITED(status) ) // tsh job exited normally
		{
			printf("Job [%d] (%d) Exited Normally Signal %d\n", child_job->jid,
					child_job->pid, WTERMSIG(status));
			deletejob(jobs, child_pid); // delete tsh job
		}
		else if( WIFSIGNALED(status) ) // tsh job term b/c unhandled error
		{
			printf("Job [%d] (%d) Terminated by Signal %d\n",
					child_job->jid, child_job->pid, WTERMSIG(status));
			deletejob(jobs, child_pid);
		}
		else if( WIFSTOPPED(status) ) // tsh job stopped
		{
			printf("Job [%d] (%d) Stopped by Signal %d\n", child_job->jid,
					child_job->pid, WIFSTOPPED(status));
			child_job->state = ST; // update jobs array
		}	
	}

	// child_pid == -1, waitpid had error
	if( errno == ECHILD ) // main shell has no children
		// this is OK b/c it may be that no jobs run yet
	{
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

	if( fg_pid == 0 ) // No current jobs
	{
		printf("No Foreground Jobs\n");
		return;
	}
	else
	{	
		if( Kill(-fg_pid, SIGINT) ) // kill this job and all others in its process group
		// successfully sent signal
		{
			if( verbose )
				printf("Job [%d] (%d) Terminated by Ctrl+C\n", fg_job->jid, fg_pid);
		}
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

	if( fg_pid == 0 )
	{
		printf("No Foreground Jobs\n");
		return;
	}
	else
	{
		if( Kill(-fg_pid, SIGTSTP) ) // send signal to stop job
		// successfully sent signal
		{
			if( verbose )
				printf("Job [%d] (%d) Stopped by Ctrl+Z\n", fg_job->jid, fg_job->pid);
			fg_job->state = ST;
		}
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
        printf("Added job [%d] %d %s\n",
         jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
       }
      return 1;
     }
   }
  printf("Tried to create too many jobs\n");
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
        printf("listjobs: Internal error: job[%d].state=%d\n",
        i, jobs[i].state);
       }
      printf("%s", jobs[i].cmdline);
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
* unix_error - unix-style error routine
*/
void unix_error(char *msg)
 {
  fprintf(stdout, "%s: %s\n", msg, strerror(errno));
  exit(1);
 }

/*
* app_error - application-style error routine
*/
void app_error(char *msg)
 {
  fprintf(stdout, "%s\n", msg);
  exit(1);
 }

/*
* Signal - wrapper for the sigaction function
*/
handler_t *Signal(int signum, handler_t *handler)
 {
  struct sigaction action, old_action;

  action.sa_handler = handler;
  sigemptyset(&action.sa_mask);   /* block sigs of type being handled */
  action.sa_flags = SA_RESTART;   /* restart syscalls if possible */

  if (sigaction(signum, &action, &old_action) < 0)
  unix_error("Signal error");
  return (old_action.sa_handler);
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



