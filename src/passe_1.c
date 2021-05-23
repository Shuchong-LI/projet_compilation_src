#include <stdlib.h>
#include <stdio.h>

#include "defs.h"
#include "passe_1.h"
#include "miniccutils.h"


extern int trace_level;

node_type current_node_type = TYPE_NONE;
node_t lval = NULL;
int is_lval_in_decl = 0;
int is_rval_in_decl = 0;
int is_fun_decl = 0;
int is_global = 0;
int taille_pile = 0;

/*
faire une fonction d'erreur avec le numero de la ligne  de ce type2

faire les tests en conséquence

void yyerror(node_t * program_root, char * s) {
    fprintf(stderr, "Error line %d: %s\n", yylineno, s);
    exit(1);
}

*/
// à améliorer
void printerror(){
	fprintf(stderr, "erreur type_op");
    exit(1);
}

node_type type_op_unaire(node_nature operateur, node_type type) {
	switch (operateur) {
		case NODE_MINUS: NODE_BNOT:
		if (type != TYPE_INT){
			printerror();
		}
		return type;
		case NODE_NOT:
		if (type != TYPE_BOOL){
			printerror();
		}
		return type;
		default:
		fprintf(stderr, "operateur incompatible avec operation unaire");
	    exit(1);

	}
	// if (operateur == NODE_MINUS){
	// 	if (type != TYPE_INT){
	//
	// 	}
	// }
}

node_type type_op_binaire(node_nature operateur, node_type type1, node_type type2) {
	switch (operateur) {
		case NODE_PLUS: NODE_MINUS: NODE_MUL: NODE_DIV: NODE_MOD: NODE_BAND: NODE_BOR: NODE_BXOR: NODE_SLL: NODE_SRL: NODE_SRA:

	}
	return type;
}

void analyse_passe_1(node_t root) {
	int32_t tmp_offset;
	if (root == NULL)
		return;

	switch (root->nature) {
	case NODE_PROGRAM:
		push_global_context();
		is_global = 1;
		analyse_passe_1(root->opr[0]);
		is_global = 0;
		analyse_passe_1(root->opr[1]);
		pop_context();
		break;

	case NODE_FUNC:
		is_fun_decl = 1;
		reset_env_current_offset();
		analyse_passe_1(root->opr[0]);
		analyse_passe_1(root->opr[1]);
		is_fun_decl = 0;
		analyse_passe_1(root->opr[2]);
		taille_pile = get_env_current_offset();
		root->offset = taille_pile;
		break;

	case NODE_TYPE:
		current_node_type = root->type;
		break;

	case NODE_IDENT:
		// lval in declaration
		if (is_lval_in_decl) {
			lval = root;
			tmp_offset = env_add_element(root->ident, root);
			if (tmp_offset < 0) {
				fprintf(stderr, "Erreur : variable %s deja declare\n", root->ident);
				goto free_program_after_error;
			}
			root->offset = tmp_offset;
			root->type = current_node_type;
			if (root->type == TYPE_VOID) {
				fprintf(stderr, "Error : void is not an allowed variable type\n");
				goto free_program_after_error;
			}
			root->global_decl = is_global;
		} else if (is_fun_decl) {
			if (strcmp(root->ident, "main") == 0 && current_node_type != TYPE_VOID) {
				fprintf(stderr, "Erreur : main should be void type\n");
				goto free_program_after_error;
			}
			root->type = current_node_type;
			root->global_decl = is_global;
		} else {
			node_t tmp_node = (node_t) get_decl_node(root->ident);
			// Si la variable n'a pas ete declare
			if (tmp_node == NULL) {
				fprintf(stderr, "Erreur : variable %s non declare\n", root->ident);
				goto free_program_after_error;
			}
			root->decl_node = tmp_node;
			root->type = root->decl_node->type;
			root->offset = root->decl_node->offset;
		}
		break;

	case NODE_BLOCK:
		push_context();
		analyse_passe_1(root->opr[0]);
		analyse_passe_1(root->opr[1]);
		pop_context();
		break;

	case NODE_DECLS:
		analyse_passe_1(root->opr[0]);
		analyse_passe_1(root->opr[1]);
		break;

	case NODE_DECL:
		is_lval_in_decl = 1;
		analyse_passe_1(root->opr[0]);
		is_lval_in_decl = 0;
		is_rval_in_decl = 1;
		analyse_passe_1(root->opr[1]);
		is_rval_in_decl = 0;
		break;

	case NODE_LIST:
		analyse_passe_1(root->opr[0]);
		analyse_passe_1(root->opr[1]);
		break;

	case NODE_STRINGVAL:
		if (is_rval_in_decl) {
			printf("Erreur : string litteral cannot be assigned to variable\n");
			goto free_program_after_error;
		}
		root->offset = add_string(root->str);
		break;

	case NODE_INTVAL:
		if (is_rval_in_decl) {
			if (lval->type != TYPE_INT) {
				printf("Erreur : %s is not int\n", lval->ident);
				goto free_program_after_error;
			}
			lval->value = root->value;
		}
		break;
	
	case NODE_BOOLVAL:
		if (is_rval_in_decl) {
			if (current_node_type != TYPE_BOOL) {
				printf("Erreur : %s is not bool\n", lval->ident);
				goto free_program_after_error;
			}
			lval->value = root->value;
		}
		break;

	case NODE_PRINT:
		analyse_passe_1(root->opr[0]);
		break;

	case NODE_IF:

		break;
	default:
	}
	return;

free_program_after_error: //TODO: A modifie
	free_global_strings();
	free_nodes();
	exit(1);
}
