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
void printerror(node_t node){
	fprintf(stderr, "Error line %d: operateur incompatible avec operation\n", node->lineno);
	exit(1);
}

node_type type_op_unaire(node_nature operateur, node_t noeud) {
	switch (operateur) {
	case NODE_MINUS: NODE_BNOT:
		if (noeud->type != TYPE_INT){
			printerror(noeud);
		}
		return TYPE_INT;
	case NODE_NOT:
		if (noeud->type != TYPE_BOOL){
			printerror(noeud);
		}
		return TYPE_BOOL;
	default:
		printerror(noeud);
	}
}

node_type type_op_binaire(node_nature operateur, node_t n1, node_t n2) {
	switch (operateur) {
		case NODE_PLUS: NODE_MINUS: NODE_MUL: NODE_DIV: NODE_MOD: NODE_BAND: NODE_BOR: NODE_BXOR: NODE_SLL: NODE_SRL: NODE_SRA:
			if (n1->type != TYPE_INT){
				printerror(n1);
			}
			if (n2->type != TYPE_INT){
				printerror(n2);
			}
			return TYPE_INT;
		case NODE_EQ: NODE_LT: NODE_GT: NODE_LE: NODE_GE:
			if (n1->type != TYPE_INT){
				printerror(n1);
			}
			if (n2->type != TYPE_INT){
				printerror(n2);
			}
			return TYPE_BOOL;
		case NODE_AND: NODE_OR: NODE_EQ:
			if (n1->type != TYPE_BOOL){
				printerror(n1);
			}
			if (n2->type != TYPE_BOOL){
				printerror(n2);
			}
			return TYPE_BOOL;
		case NODE_NE: // deux possibilités pour ce node : bool-bool et int-int
			if ((n1->type == TYPE_INT) && (n2->type == TYPE_INT)){
				return TYPE_BOOL;
			}
			if ((n1->type == TYPE_BOOL) && (n2->type == TYPE_BOOL)){
				return TYPE_BOOL;
			}
			printerror(n1);
		default:
			printerror(n1);
	}
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
		reset_temporary_max_offset();
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
			root->type = tmp_node->type;
			root->offset = tmp_node->offset;
			root->global_decl = tmp_node->global_decl;
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

	case NODE_MINUS: NODE_BNOT: NODE_NOT:
			type_op_unaire(root->nature, root);
			break;

	default:
		break;
	}
	return;

free_program_after_error: //TODO: A modifie
	free_global_strings();
	free_nodes();
	exit(1);
}
