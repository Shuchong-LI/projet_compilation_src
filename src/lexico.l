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
int nbr_registre_max = 8;

#if YYDEBUG
extern int yydebug;
#endif

%}

%option yylineno


LETTRE          [a-zA-Z]
/* A completer ; fausses definitions pour passer la compilation */
CHIFFRE_NON_NUL [1-9]
CHIFFRE         [0-9]
ENTIER_DEC      0|{CHIFFRE_NON_NUL}{CHIFFRE}*
LETTRE_HEXA     [a-fA-F]
ENTIER_HEXA     0x({CHIFFRE}|{LETTRE_HEXA})+
ENTIER          {ENTIER_DEC}|{ENTIER_HEXA}
IDF             [a-zA-Z]+[a-zA-Z0-9]*
CHAINE          \"((\\\")|[^\"])*\"
COMMENTAIRE     \/\/.*

%%

"void"          return TOK_VOID;
"int"           return TOK_INT;
"bool"          return TOK_BOOL;
"true"          return TOK_TRUE;
"false"         return TOK_FALSE;
"if"            return TOK_IF;
"then"          return TOK_THEN;
"else"          return TOK_ELSE;
"while"         return TOK_WHILE;
"for"           return TOK_FOR;
"do"            return TOK_DO;
"print"         return TOK_PRINT;

"+"             return TOK_PLUS;
"-"             return TOK_MINUS;
"*"             return TOK_MUL;
"/"             return TOK_DIV;
"%"             return TOK_MOD;
">"             return TOK_GT;
"<"             return TOK_LT;
"!"             return TOK_NOT;
"~"             return TOK_BNOT;
"&"             return TOK_BAND;
"|"             return TOK_BOR;
"^"             return TOK_BXOR;
"="             return TOK_AFFECT;
";"             return TOK_SEMICOL;
","             return TOK_COMMA;
"("             return TOK_LPAR;
")"             return TOK_RPAR;
"{"             return TOK_LACC;
"}"             return TOK_RACC;
">>"            return TOK_SRA;
">>>"           return TOK_SRL;
"<<"            return TOK_SLL;
">="            return TOK_GE;
"<="            return TOK_LE;
"=="            return TOK_EQ;
"!="            return TOK_NE;
"&&"            return TOK_AND;
"||"            return TOK_OR;

{IDF} {
                yylval.strval = strdup(yytext);
                return TOK_IDENT;
}

{ENTIER} {
                /* A completer : affecter yylval.intval et faire des verifications */
                long tmp;
                if (yytext[1] == 'x')
                    tmp = strtol(yytext, NULL, 16);
                else
                    tmp = atol(yytext);
                if (tmp > 0xFFFFFFFF) {
                    fprintf(stderr, "Error line %d : Integer out of range\n", yylineno);
                    yylex_destroy();
                    exit(1);
                }
                yylval.intval = tmp;
                return TOK_INTVAL;
}

{CHAINE} {
                /* A completer : affecter yylval.strval */
                yylval.strval = strdup(yytext);
                return TOK_STRING;
}

{COMMENTAIRE} {
}


[ \t\n]+

. {
                fprintf(stderr, "Error line %d: Lexical error\n", yylineno);
                yylex_destroy();
                exit(1);
}


%%

int yywrap(void) {
    return 1;
}


int main(int argc, char ** argv) {

    if (argc < 2) {
        printf("Usage : ./minicc <options> <infile>\n\n");
		printf("Options disponibles :\n");
		printf("-b : banniere\n");
		printf("-o <filename> : nom du fichier assembleur produit\n");
		printf("-t <int> : niveau de trace (entre 0 et 5)\n");
		printf("-r <int> : nombre maximum de registres (entre 4 et 8)\n");
		printf("-s : arreter la compilation apres l'analyse syntaxique\n");
		printf("-v : arreter la compilation apres la passe de verifications\n");
        return 0;
    }

    node_t program_root;
    parse_args(argc, argv);
    #if YYDEBUG
    yydebug = 1;
    #endif

    yyin = fopen(infile, "r");
    if (yyin == NULL) {
        fprintf(stderr, "Error : impossible d'ouvrir le fichier %s\n", infile);
        exit(1);
    }
    yyparse(&program_root);
    fclose(yyin);
    analyse_tree(program_root);
    yylex_destroy();
    return 0;
}
