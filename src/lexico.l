%{
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "defs.h"
#include "common.h"

#include "y.tab.h"

int yyparse(node_t * program_root);
void analyse_tree(node_t root);


char * infile = NULL;
char * outfile = DEFAULT_OUTFILE;
bool stop_after_syntax = false;
bool stop_after_verif = false;

#if YYDEBUG
extern int yydebug;
#endif


%}

%option yylineno


LETTRE          [a-zA-Z]
/* A completer ; fausses definitions pour passer la compilation */
IDF             [a-zA-Z]+[a-zA-Z0-9]*
ENTIER          0|[1-9]+[0-9]*|0x[0-9a-fA-F]+
CHAINE          \"((\\\")|[^\"])*\"
COMMENTAIRE     \/\/.*

%%

"void"          return TOK_VOID;






{IDF} {
                yylval.strval = strdup(yytext);
                return TOK_IDENT;
}

{ENTIER} {
                /* A completer : affecter yylval.intval et faire des verifications */
                return TOK_INTVAL;
}

{CHAINE} {
                /* A completer : affecter yylval.strval */
                return TOK_STRING;
}

{COMMENTAIRE} {
}


[ \t\n]+

. {
                fprintf(stderr, "Error line %d: Lexical error\n", yylineno);
                exit(1);
}


%%

int yywrap(void) {
    return 1;
}


int main(int argc, char ** argv) {
    node_t program_root;
    parse_args(argc, argv);
    #if YYDEBUG
    yydebug = 1;
    #endif
    
    yyin = fopen(infile, "r");
    yyparse(&program_root);
    fclose(yyin);
    analyse_tree(program_root);
    yylex_destroy();
    return 0;
}




