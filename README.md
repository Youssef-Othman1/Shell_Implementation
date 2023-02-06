# Shell_Implementation
Implementing a shell using Lex and Yacc

The main goal is to build a mini shell and execute some simple shell commands.
Mainly we use Lex and Yacc to build the scanner and parser for the shell.

Build the shell program by typing : make

To run it type:  ./shell

Then type commands like:
                ls -al
                ls -al aaa bbb > out
to check the output. 

## Commands supported by shell:
-Taking input from file  
-Overwriting an existent file for output  
-Appending to an existent file for output  
-Creating an output file  
-Piping (No upper limit for number of simple commands)  
-Directory changing command  
-Error file to be appended (Created in the first time) when a command error is encountered  
-CTRL-C is ignored when encountered. exit serves as an internal command used to exit the shell when typed  
-Parsing occurs when no spaces between special characters exist  
-Log file exists for every command occuring  
