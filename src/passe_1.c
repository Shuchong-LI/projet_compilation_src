#include <stdlib.h>
#include <stdio.h>

#include "defs.h"
#include "passe_1.h"
#include "miniccutils.h"


extern int trace_level;

node_type current_node_type = TYPE_VOID;
int is_lval_in_decl = 0;
int is_fun_decl = 0;
int is_global = 0;
int taille_pile = 0;

void analyse_passe_1(node_t root) {
	int32_t tmp_offset;
	node_t tmp_node;
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
				goto free_programm_after_error;
			}
			root->offset = tmp_offset;
			root->type = current_node_type;
			if (root->type == TYPE_VOID) {
				fprintf(stderr, "Error : void is not an allowed variable type\n");
				goto free_programm_after_error;
			}
			root->global_decl = is_global;
		} else if (is_fun_decl) {
			if (strcmp(root->ident, "main") == 0 && current_node_type != TYPE_VOID) {
				fprintf(stderr, "Erreur : main should be void type\n");
				goto free_programm_after_error;
			}
			tmp_offset = env_add_element(root->ident, root);
			if (tmp_offset < 0) {
				fprintf(stderr, "Erreur : main deja declare\n");
				goto free_programm_after_error;
			}
			root->offset = tmp_offset;
			root->type = current_node_type;
			root->global_decl = is_global;
		} else {
			printf("sizeof node_t: %d\n", sizeof(node_t));
			printf("0x%llx\n", get_decl_node(root->ident));
			node_t tmp_node = (node_t) get_decl_node(root->ident);
			// Si la variable n'a pas ete declare
			if (tmp_node == NULL) {
				fprintf(stderr, "Erreur : variable %s non declare\n", root->ident);
				goto free_programm_after_error;
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
		analyse_passe_1(root->opr[1]);
		break;

	case NODE_LIST:
		analyse_passe_1(root->opr[0]);
		analyse_passe_1(root->opr[1]);
		break;

	case NODE_STRINGVAL:
		//root->offset = (root->str);
		break;

	case NODE_PRINT:
		analyse_passe_1(root->opr[0]);
		break;
	default:
		break;
	}
	return;

free_programm_after_error: //TODO: A modifie
	free_global_strings();
	free_nodes();
	exit(1);
}

