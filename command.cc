#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include "command.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>

char CWD[256];
SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}
	
	if ( _inputFile ) {
		free( _inputFile );
	}
	if ( _errFile ) {
		free( _errFile );
	}
	else if ( _outFile ) {
		free( _outFile );
	}	
	
	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
Command::execute()
{
	int fdesc;
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}
	print();
	if (strcmp(_simpleCommands[0]->_arguments[0],"cd")==0)
	{	
		int v;
		if (_simpleCommands[0]->_numberOfArguments==1)
			v=chdir("/home");		
		else{
		 	v=chdir(_simpleCommands[0]->_arguments[1]);
			}
		if (v==-1){
			printf("No such file/directory \n");
		} 
	}
	else{
	int defaultin = dup( 0 ); // Default file Descriptor for stdin
	int defaultout = dup( 1 ); // Default file Descriptor for stdout
	int defaulterr = dup( 2 ); // Default file Descriptor for stderr
	// as it might be changed by the parent during execution
	int infd=dup(defaultin); // variable file descriptor (which is set to a certain file when the command requires a different input source rather than stdin otherwise it will be stdin).
	// the next if else statment introduce the previous sentence.
	if(_inputFile){
		infd=open(_inputFile,O_RDONLY,0666); // set file variable file descriptor for input by the required input file requested by user command in my shell.  
	}
	if(_errFile){
		int errfd=open(_errFile,O_RDWR|O_APPEND|O_CREAT,0666);
		dup2(errfd,2);
		close(errfd);
	}
	int pid;
	int pipefd[2];
	int outfd;// variable file descriptor (which is set to a certain file when the command requires a different output destination rather than stdout otherwise it will be stdout).
	for(int i=0;i<_numberOfSimpleCommands;i++){
		dup2(infd, 0);
		close( infd );
		if(i == _numberOfSimpleCommands-1){
			if (_outFile){ // just like the previous illustration regarding the input.
			if(_writeMode==0){
				outfd = creat(_outFile, 0666 );
			}
			else if(_writeMode==1){
				outfd = open(_outFile,O_RDWR|O_APPEND|O_CREAT,0666); 
			} 
						
			}
			else{
				outfd = dup(defaultout);
			}
			}
		else {
			pipe(pipefd);
			infd=pipefd[0];
			outfd=pipefd[1];
		}
		dup2(outfd,1);
		close(outfd);	
		pid = fork();
		if ( pid == -1 ) {
			perror( "ls forking error\n");
			exit( 2 );
		}
		if (pid == 0) {
			//Child
			execvp(_simpleCommands[i]->_arguments[0],_simpleCommands[i]->_arguments);
			char ErrorMsg[100]= "execvp() error at ";
			time_t timeNow=time(NULL);
			strcat(ErrorMsg,ctime(&timeNow));
   			perror(ErrorMsg);
        		exit(2);
	}
	}
	// Restore input, output, and error
	dup2( defaultin, 0 );
	dup2( defaultout, 1 );
	dup2( defaulterr, 2 );
	// Close file descriptors that are not needed
 	close( defaultin );
	close( defaultout );
	close( defaulterr );
	// Wait for last process in the pipe line
	if (_background==0)
	waitpid( pid, 0, 0 );
	}
	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation

void
Command::prompt()
{
	printf("\nmyshell> ");
	fflush(stdout);
}
void sigintHandler(int sig_num)
{
    printf("\n\t\tProgram is not terminated using Ctrl+C \n");
    printf("\nmyshell> ");   
    fflush(stdout);
}
void handle_sigchld(int sig) {
   int defaultout1=dup(1);
   int outfd = open(CWD,O_RDWR|O_APPEND|O_CREAT,0666);
   dup2( outfd, 1 );
   time_t timeNow=time(NULL);
   printf("Process terminated %s",ctime(&timeNow));
   close(outfd);
   fflush(stdout);
   dup2 (defaultout1,1);
   close(defaultout1);
}
Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{	
	getcwd(CWD, sizeof(CWD));
	strcat(CWD,"/ChildrenTerminated.log");
	signal(SIGINT, sigintHandler);
	signal(SIGCHLD, handle_sigchld);
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}

