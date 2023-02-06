%{

#include <string.h>
#include "y.tab.h"

%}

%%

\n 	{
		return NEWLINE;
	}
[ \t] 	{
		/* Discard spaces and tabs */
     	}	
"2>" 	{
		return ERRORFILE;
	}
"&>>" 	{
		return ERRORFILENEEDEDAPP;
	}
"&>" 	{
		return ERRORFILENEEDEDOVER;
	}
"&"	{
		return AMPERSAND;
	}						
"exit"	{
		return EXIT;
	}	
">>" 	{
		return APPEND;
	}
">" 	{
		return GREAT;
	}	
"<" 	{
		return INPUT;
	}
"|" 	{
		return PIPE;
	}

[^\|\>\<\& \t\n][^\|\>\<\& \t\n]* {
   		yylval.string_val = strdup(yytext);
    		return WORD;
	}
.	{
		/* Invalid character in input */
		return NOTOKEN;
	}

%%
	
