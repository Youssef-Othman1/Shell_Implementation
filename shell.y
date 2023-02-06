%token	<string_val> WORD

%token 	NOTOKEN AMPERSAND GREAT ERRORFILE ERRORFILENEEDEDAPP ERRORFILENEEDEDOVER APPEND INPUT PIPE EXIT NEWLINE 

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;

simple_command:	
	piping iomodifier_opt_2 NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	|NEWLINE {
		Command::_currentCommand.prompt();
	} 
	|
	EXIT {
		printf("\t\t\tGood bye!!\n");
		exit(0);
	}
	| error NEWLINE { yyerrok; }
	;
command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;
arg_list:
	arg_list argument
	| /* can be empty */
	;
argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;
iomodifier_opt_2:
	iomodifier_opt_2 iomodifier_opt 
	|
	;	
iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output to \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._writeMode=0; 
	}
	|ERRORFILE WORD {
		printf("   Yacc: print error to \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
	}
	|ERRORFILENEEDEDAPP WORD{
		printf("   Yacc: print output/error to \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._writeMode=1; 
	}
	|ERRORFILENEEDEDOVER WORD{
		printf("   Yacc: print output/error to \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._writeMode=0; 
	}
	|APPEND WORD {
		printf("   Yacc: append output to \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._writeMode=1; 
	}
	|INPUT WORD {
		printf("   Yacc: take input from \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	|AMPERSAND {
		Command::_currentCommand._background=1;
	}
	;
	
piping:
	piping PIPE command_and_args
	|command_and_args
	;	

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
