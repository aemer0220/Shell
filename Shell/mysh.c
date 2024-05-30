/*
 * Alexandra Emerson
 *
 * CS441/541: Project 3
 *
 * Last Modified: 5/3/24
 */
#include "mysh.h"

int main(int argc, char * argv[]) {
    int ret;
	/* Parse command line arguments */
    if (0 != (ret = parse_args_main(argc, argv))) {
        fprintf(stderr, "Error: Invalid command line!\n");
        return -1;
    }
	/* If batch mode, process files*/
    if (TRUE == is_batch) {
		//debug mode
		if (TRUE == is_debug) {
            fprintf(stdout,"Batch Mode!\n");
        }
		
        if (0 != (ret = batch_mode())) {
            fprintf(stderr, "Error: Batch mode returned a failure!\n");
        }
		
	 /* Otherwise, interactive mode*/
    } else if (FALSE == is_batch) {
		//debug mode
		if (TRUE == is_debug) {
            fprintf(stdout,"Interactive Mode!\n");
        }
        if (0 != (ret = interactive_mode())) {
            fprintf(stderr, "Error: Interactive mode returned a failure!\n");
        }

	 /* Otherwise, fail. Should not reach here.*/
    } else {
        fprintf(stderr, "Error: Unknown execution mode!\n");
        return -1;
    }
	
    return 0;
}

int parse_args_main(int argc, char ** argv) {
    int i;
	
    if (argc > 1) {
		is_batch = TRUE; //set batch mode

		num_files = argc - 1;
        files = malloc(sizeof(char *) * num_files); //allocate array of files names

		for (i = 0; i < num_files; i++) {
            files[i] = malloc(sizeof(char) * strlen(argv[i + 1]));
            strcpy(files[i], argv[i + 1]);
        }	
    }
	else if (argc == 1) { //no argument provided, so interactive mode
        is_batch = FALSE;
    }
	
    return 0;
}

int batch_mode() {
	char **commands;
	int i, j, start, end;
	FILE *file;
	ssize_t line;

	char *buf;
    size_t length = 0;

	for (i = 0; i < num_files; i++) {
		file = fopen(files[i], "r"); //open each file in the files array
		//if file does not exist
		if (file == NULL) {
            fprintf(stderr, "ERROR: Unable to open file\n");
        }
		//read file one line at a time, until we reach EOF
        while ((line = getline(&buf, &length, file)) != EOF) {
            buf[strlen(buf) - 1] = '\0';

			commands = split(buf, ";");
            start = total_history;

			i = 0;
			while (commands[i] != NULL) {
                create_job(commands[i]); //create and add jobs corresponding to the split commands
                i++;
            }
			
            end = total_history;
            for (j = start; j < end; j++) {
                launch_job(&jobs[j]);
            }
        }
		
        fclose(file);
        file = NULL;
    }
	
    builtin_exit();

	//free our commands
	while(*commands != NULL){
        free(*commands);
        commands++;
    }
	free(commands);

	return 0;
}

int interactive_mode() {
    char **commands;
    int i, j, start, end;
    char buf[MAX_COMMAND_LINE];
	
    setbuf(stdout, NULL);
	fprintf(stdout, PROMPT); //print prompt
	
    while (fgets(buf, MAX_COMMAND_LINE, stdin) != NULL) {
        buf[strlen(buf) - 1] = '\0';
        commands = split(buf, ";");
        
        start = total_history; //start is total history before creating jobs

		i = 0;
        while (commands[i] != NULL) {
            create_job(commands[i]);
            i++;
        }
		
        end = total_history; //end is total history after creating jobs

		for (j = start; j < end; j++) {
            launch_job(&jobs[j]); //launch each job created
        }
		
        fprintf(stdout, PROMPT); //print prompt again
    }
	
    builtin_exit(); //exit when we're done

	//free each command
	while(*commands != NULL){
        free(*commands);
        commands++;
    }
    free(commands);

    return 0;
}

void create_job(char *input) {
    int i;

    char **split_input = split(input, "&"); //split input by &

	if (strlen(split_input[0]) != strlen(input)) { //if we've found a background job
		i = 0;
		while(split_input[i] != NULL){
			jobs[total_history].is_background = TRUE; //set as background job
			add_job(split_input[i]); //add each job found
			i++;
		}

		if (input[strlen(input) - 1] == '&') { //if background indicator is provided
			jobs[total_history - 1].is_background = TRUE; //ensure it is set to true
		}
		else {
			jobs[total_history - 1].is_background = FALSE; //otherwise it is a foreground job
		}
		return; //terminate method
    }

	//will only get here if it is a background job
    jobs[total_history].is_background = FALSE;
	add_job(input);
}

void add_job(char *input){ //adds an individual job, increments total history
	char **split_job, **fc_split; //fc_split for redirection part
    char *full_command, *fc;      //fc for redirection part
    int command_length, h;

	h = total_history;

	//set attributes
	if(jobs[h].is_background == FALSE){
		command_length = strlen(input)+1;
		jobs[h].full_command = malloc(sizeof(char) * command_length);
	}
	
	split_job = split(input, " ");

	full_command = trim_whitespace(input);
	jobs[h].full_command = full_command; //assign full_command
	
	jobs[h].binary = split_job[0];
	jobs[h].argv = split_job;
	jobs[h].argc = get_num_args(split_job);
	
	jobs[h].is_done = 0;
	jobs[h].done_ack = 0;
	
	//Take care of redirection
	if (strchr(jobs[h].full_command, '<')) { //check < redirection
		
		fc = strdup(full_command);
		fc_split = split(fc, "< ");
		
		jobs[h].bin = strdup(fc_split[0]); //used for forking with redirection
		jobs[h].path = strdup(fc_split[1]); //used for opening the file for forking
		jobs[h].redirection = 1;  //for O_RDONLY
		
	} else if (strchr(jobs[h].full_command, '>')) { //check > redirection
		fc = strdup(full_command);
		fc_split = split(fc, "> ");
		
		jobs[h].bin = strdup(fc_split[0]); //used for forking with redirection
		jobs[h].path = strdup(fc_split[1]); //used for opening the file for forking
		jobs[h].redirection = 2; //for O_CREATE | O_TRUNC | O_WRONLY

	}
		total_history++; //increase total jobs
}


int launch_job(struct job_t * job) {
    int pid, file;
    char *binary;
    char **split_binary;
	int status = 0;
    pid_t c_pid = 0;
	//check for built-ins first
	if (strcmp(job->binary, "exit") == 0) {
		builtin_exit();
    }
	else if (strcmp(job->binary, "jobs") == 0) {
        builtin_jobs();
    }
	else if (strcmp(job->binary, "history") == 0) {
        builtin_history();
    }
	else if (strcmp(job->binary, "wait") == 0) {
        builtin_wait();
    }
	else if (strcmp(job->binary, "fg") == 0) {
        if (job->argc > 1) {
            builtin_fg_num(atoi(job->argv[1]));
        }

		else {
            builtin_fg();
        }
    }
	//otherwise, fork
	else {
		total_jobs++;
      
        c_pid = fork(); //create process

		if (c_pid == 0) { //if we're the child
			//get the pid
            job->pid = getpid();
			//check redirection
			if (job->redirection == 1) {
				binary = strdup(job->bin); //get the bin
				split_binary = split(binary, " ");

				file = open(job->path, O_RDONLY); //open based on path
				dup2(file, STDIN_FILENO); //create copy of file descriptor

				close(file);
				execvp(split_binary[0], split_binary); //call exec

			} else if (job->redirection == 2) {
				binary = strdup(job->bin); //get the bin
				split_binary = split(binary, " ");

				file = open(job->path, O_CREAT | O_TRUNC | O_WRONLY);
				dup2(file, STDOUT_FILENO); //create copy of file descriptor

				close(file);
				execvp(split_binary[0], split_binary); //call exec
            }

			else {
                execvp(job->binary, job->argv);
            }
			
            fprintf(stderr, "ERROR: Command not found.\n");
            exit(1);
        }
	    
		else if (c_pid > 0){ //if we're the parent
            job->pid = c_pid;
            if (job->is_background == FALSE) {

				pid = waitpid(c_pid, &status, WUNTRACED); //return with status info if the child has terminated

				if (WIFEXITED(status)) { //check status - make sure it's good
                    update_status(pid, 0); //update status to done
                }
            }
			
			else {
                total_jobs_bg++;
                signal(SIGCHLD, signal_child); //signal child (background) process, use WNOHANG
            }
        }

		
		else { //failure
            fprintf(stdout,"ERROR: fork() failed\n");
            return -1;
        }
    }
	
    return 0;
}

 
void signal_child(int signal) {
	pid_t c_pid;
	int status;
	
    assert(signal == SIGCHLD);
	while ((c_pid = waitpid(-1, &status, WNOHANG)) > 0) { //return the c_pid
        update_status(c_pid, 0); //update status
    }
}

int builtin_jobs() {
    int i;
	char *status;

	for (i = 0; i < total_history; i++) {
        if ((jobs[i].is_background == 1) && (jobs[i].done_ack == 0)) { //if it's a background job and we haven't acknowledged a 'done' state

			if(jobs[i].is_done == 0){
				status = "Running";
			}
			else{
				status = "Done";
			}

			fprintf(stdout, "[%d]  %s %5s\n", i + 1, status, jobs[i].full_command);

			if (jobs[i].is_done) {
                jobs[i].done_ack = 1;
            }
        }
    }
    return 0;
}

int builtin_history() {
    int i;
	
    for (i = 0; i < total_history; i++) { //print all jobs in history
        if (jobs[i].is_background == 1) {
            fprintf(stdout, "%4d  %s &\n", i + 1, jobs[i].full_command);
        }

		else {
            fprintf(stdout, "%4d  %s\n", i + 1, jobs[i].full_command);
        }
    }
    return 0;
}

int builtin_exit() {
    int num_waiting = 0;
    int i;
    	
    for(i = 0; i < total_history; i++){
        if((jobs[i].is_background == 1) && jobs[i].is_done == 0){
            num_waiting++; //find all jobs that are in the back ground and currently running
        }
    }
	
    if (num_waiting > 0) {
        fprintf(stdout,"Waiting on %d jobs to complete.\n", num_waiting);
        builtin_wait(); //wait for the background jobs to complete
    }
	
	//print statistics
    fprintf(stdout,"-------------------------------\n");
    fprintf(stdout,"Total number of jobs               = %d\n", total_jobs);
    fprintf(stdout,"Total number of jobs in history    = %d\n", total_history);
    fprintf(stdout,"Total number of jobs in background = %d\n", total_jobs_bg);
    
    exit(0);
}

int builtin_wait() {
	int s = 0;
	pid_t wait_pid;

	while ((wait_pid = wait(&s)) > 0) {
        update_status(wait_pid, 0); //update status has jobs get done
    }
	
    return 1;
}

void update_status(int pid, int foreground) {
    int i;
	
    for (i = 0; i < total_history; i++) {
        if (jobs[i].pid == pid) {
            jobs[i].is_done = 1; //update status to done
			
            if (foreground == 1) { //if we're here to mark it as a foreground job
                jobs[i].is_background = 0;
                total_jobs_bg--; //decrement background jobs count
            }
        }
    }
}

int builtin_fg() { //waits for default background job to finish
    int i;
	
    for (i = total_history - 1; i >= 0; i--) {

		if (jobs[i].is_background == 1) {
            builtin_fg_num(i + 1); //call specific fg method for each background job

			return 1;
        }
    }
    return 0;
}

int builtin_fg_num(int pid) { //waits for specified background job to finish

	int status = 0;

	if (jobs[pid - 1].is_background == 0) { //check if not a background job
        fprintf(stdout,"Process [%d] is not a background job\n", pid);
        return 0;
    }
	
    if (jobs[pid - 1].is_done == 1) { //check if done
        fprintf(stdout,"Process [%d] is already completed\n", pid);
        return 0;
    }
	
    waitpid(jobs[pid - 1].pid, &status, WUNTRACED);

	if (WIFEXITED(status)) {
        update_status(jobs[pid - 1].pid, 1);
        return 1;
    }
    return 0;
}


char * trim_whitespace(char *string) {
	char *trimmed = strdup(string); //create new string from original
	int i;
	
    if (string[strlen(string) - 1] == ' ') { //check last character
        trimmed[strlen(string) - 1] = '\0'; //put null terminator if it's a space
    }
	
    if (string[0] == ' ') { //check first chracter and onward if it's a space
        for (i = 1; i < strlen(trimmed); i++) {
            trimmed[i - 1] = string[i];
        }

		trimmed[i - 1] = '\0';
    }
	
    return trimmed; //return result
}

char ** split(char *input, char *delimeter) {
    char *string = strdup(input);
    char **array = malloc(sizeof(char *) * MAX_COMMAND_LINE);
    char *token = strtok(string, delimeter);

	int i = 0;
    while (token != NULL) { //while we still have tokens
        array[i] = token;
        token = strtok(NULL, delimeter); //split input into tokens by delimiter
		i++;
	}
	
    return array; //return array of split input
}


int get_num_args(char **arg) {
	int i = 0;
	while (arg[i] != NULL) {
        i++; //get the collective number of arguments
    }
	
    return i;
}

//Free a job from the jobs list
void free_job(struct job_t * job){
	//need to free our jobs list
	if(job){
		//free attributes
        free(job->full_command);
        free(job->binary);
        free(job->path);
        free(job->bin);
    }
}
