/*
 * Alexandra Emerson
 *
 * CS441/541: Project 3
 *
 * Last Modified: 5/1/24
 */
#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
#include <fcntl.h>
/* For fork, exec, sleep */
#include <sys/types.h>
#include <unistd.h>
/* For waitpid */
#include <sys/wait.h>

/******************************
 * Defines
 ******************************/
#define TRUE  1
#define FALSE 0
#define MAX_COMMAND_LINE 1024
#define PROMPT ("mysh$ ")


/******************************
 * Structures
 ******************************/
/*
 * A job struct.  Feel free to change as needed.
 */
typedef struct job_t {
	int pid;
	char * full_command;
	int is_background;
	int is_done;
	int argc;
	char **argv;
	char *bin;
	char* binary;
	char* path;
	int redirection;
	int done_ack;
} job_t;

/******************************
 * Global Variables
 ******************************/

/* Array of all jobs */
job_t jobs[MAX_COMMAND_LINE];

/* Array of all file names */
char **files;
/* Number of files in batch mode */
int num_files;

/*Interactive or batch mode*/
int is_batch = FALSE;

/*Counts*/
int total_jobs_display_ctr = 0;
int total_jobs    = 0;
int total_jobs_bg = 0;
int total_history = 0;

/*
 * Debugging mode
 */
int is_debug = TRUE;

/******************************
 * Function declarations
 ******************************/
/*
 * Parse command line arguments passed to myshell upon startup.
 *
 * Parameters:
 *  argc : Number of command line arguments
 *  argv : Array of pointers to strings
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int parse_args_main(int argc, char **argv);

/*
 * Main routine for batch mode
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int batch_mode();

/*
 * Main routine for interactive mode
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int interactive_mode();

/*
 * trims leading and trailing whitespace
 *
 * Parameters:
 *  line: a string to remove whitespace from
 *
 * Returns:
 *  the resulting string 
 */

char *trim_whitespace(char *line);

/*
 * Launch a job
 *
 * Paramters:
 *  job : element of the jobs array
 *
 * Returns:
 *  0 on success
 *  Negative value on error
 */
int launch_job(struct job_t *job);

/*
 * Built-in 'exit' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_exit(void);

/*
 * Built-in 'jobs' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_jobs(void);

/*
 * Built-in 'history' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_history(void);

/*
 * Built-in 'wait' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_wait(void);

/*
 * Updates the status of the job to finished.
 * Remove job from background if specified.
 *
 * Parameters:
 *   proc_id: process id
 *   foreground: flag of whether to set job to foreground
 * Returns:
 *   None
 */
void update_status(int proc_id, int foreground);

/*
 * Built-in 'fg' command
 *
 * Parameters:
 *   None (use default behavior)
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_fg(void);

/*
 * Built-in 'fg' command
 *
 * Parameters:
 *   Job id
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_fg_num(int job_num);

/*
 * Splits a string based on a given delimiter
 *
 * Parameters:
 *    input: input string
 *    delimeter: delimter string
 * Returns:
 *    string array
 */
char ** split(char *input, char *delimeter);

/*
 * Creates a job based on if it's a background
 * or foreground job.
 *
 * Parameters:
 *    input: string input
 * Returns:
 *    None
 */
void create_job(char *input);


/*
 * Sets the attributes of the new job
 * and increases the total history.
 *
 * Parameters:
 *    input: a string (full command or first token depending
 *           on if it's one or multiple/background jobs)
 * Returns:
 *    None
 */
void add_job(char *input);

/*
 * returns the number of arguments in
 * the command line
 *
 * Parameters:
 *    arg: an array of string (arguments)
 * Returns:
 *    the number of arguments;
 *
 */
int get_num_args(char **arg);

/*
 *
 *
 */
void signal_child(int signal);


#endif /* MYSHELL_H */
