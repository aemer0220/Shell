# CS441/541 Shell Project

## Author(s):

Alexandra Emerson


## Date:

05/03/2024


## Description:

This project implements a simple shell which can be run in either an interactive mode, where the program prompts the user for input, or it can run in batch mode, where it will read commands from a given file. The program supports a number of built-in commands, such as history, wait, and others, as well as commands that require to be launched as a binary, such as ls and sleep. The program can carry out both sequential and concurrent job execution, executing jobs in the foreground and/or background as specified. The program will continue to prompt the user for input until either CTRL-D or EOF is detected.


## How to build the software

To build mysh, run the command
```
make
```

## How to use the software
You can run the program in one of two ways.  
To run the program in interactive mode, where you can supply commands one line at a time, run the command
```
./mysh
```
with no arguments.  

To run the program in batch mode, run the same command except with file names included as arguments. For example...  
```
./mysh examplefile.txt
```

## How the software was tested
The software was tested utilizing the command
```
make check
```
as well as running the given tests and my own tests. All three testing methods provided a solid understanding of how my program behaves.

My own tests are as follows:  
* test1.txt - This file tests wait.
* test2.txt - This file is utilized to a call to fg with specific arguments.
* test3.txt - This file is utilized to test redirection used with fg.
* test4.txt - This file is utilized to test how the program behaves when exit is called in the middle of the program.
* test5.txt - This file is utilized to test the wait command with fg.
## Known bugs and problem areas
No known bugs or problem areas.
