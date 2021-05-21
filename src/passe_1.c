#include <stdlib.h>
#include <stdio.h>

#include "defs.h"
#include "passe_1.h"
#include "miniccutils.h"


extern int trace_level;

node_type current_node_type = TYPE_NONE;
int is_lval_in_decl = 0;
int is_rval_in_decl = 0;
int is_fun_decl = 0;
int is_global = 0;
int taille_pile = 0;

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
		analyse_passe_1(root->opr[0]);
		analyse_passe_1(root->opr[1]);
		taille_pile = get_env_current_offset();
		is_fun_decl = 0;
		analyse_passe_1(root->opr[2]);
		taille_pile = get_env_current_offset() - taille_pile;
		root->offset = taille_pile;
		break;

	case NODE_TYPE:
		current_node_type = root->type;
		break;

	case NODE_IDENT:
		// lval in declaration
		if (is_lval_in_decl) {
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
		if (is_rval_in_decl)
			if (current_node_type != TYPE_STRING) {
				printf("Erreur : is incompatible with string\n");
				goto free_program_after_error;
			}
		//root->offset = (root->str);
		break;

	case NODE_INTVAL:
		if (is_rval_in_decl)
			if (current_node_type != TYPE_INT) {
				printf("Erreur : is incompatible with int\n");
				goto free_program_after_error;
			}
		break;
	
	case NODE_BOOLVAL:
		if (is_rval_in_decl)
			if (current_node_type != TYPE_BOOL) {
				printf("Erreur : is incompatible with bool\n");
				goto free_program_after_error;
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

