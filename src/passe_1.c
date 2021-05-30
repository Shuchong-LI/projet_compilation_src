#include <stdlib.h>
#include <stdio.h>

#include "defs.h"
#include "passe_1.h"
#include "miniccutils.h"
#include "common.h"

extern int trace_level;

node_t program_root;

node_type current_node_type = TYPE_NONE;
node_t lval = NULL;
int is_lval_in_decl = 0;
int is_rval_in_decl = 0;
int is_fun_decl = 0;
int is_global = 0;
int taille_pile = 0;

void printerror(node_t node){
	fprintf(stderr, "Error line %d: operateur incompatible avec operation.\n", node->lineno);
	exit(1);
}

node_type type_op_unaire(node_nature operateur, node_t noeud) {
	switch (operateur) {
	case NODE_UMINUS: case NODE_BNOT:
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

/*
TODO: affichage erreur : variable a of type TYPE_BOOL is initialized with an expression of
type TYPE INT. au lieu de operateur incompatible
*/
node_type type_op_binaire(node_nature operateur, node_t n1, node_t n2) {
	switch (operateur) {
		case NODE_PLUS: case NODE_MINUS: case NODE_MUL: case NODE_DIV: case NODE_MOD:
		case NODE_BAND: case NODE_BOR: case NODE_BXOR:
		case NODE_SLL: case NODE_SRL: case NODE_SRA:
			if (n1->type != TYPE_INT){
				printerror(n1);
			}
			if (n2->type != TYPE_INT){
				printerror(n2);
			}
			return TYPE_INT;
		case NODE_LT: case NODE_GT: case NODE_LE: case NODE_GE:
			if (n1->type != TYPE_INT){
				printerror(n1);
			}
			if (n2->type != TYPE_INT){
				printerror(n2);
			}
			return TYPE_BOOL;
		case NODE_AND: case NODE_OR:
			if (n1->type != TYPE_BOOL){
				printerror(n1);
			}
			if (n2->type != TYPE_BOOL){
				printerror(n2);
			}
			return TYPE_BOOL;
		case NODE_NE: case NODE_EQ: // deux possibilités pour ces nodes : bool-bool et int-int
			if ((n1->type == TYPE_INT) && (n2->type == TYPE_INT)){
				return TYPE_BOOL;
			} else if ((n1->type == TYPE_BOOL) && (n2->type == TYPE_BOOL)){
				return TYPE_BOOL;
			}
			printerror(n1);
		case NODE_AFFECT:
			if (n1->type != TYPE_VOID && n1->type == n2->type)
				return n1->type;
			else
				printerror(n1);
		default:
			printerror(n1);
	}
}

void analyse_passe_1(node_t root) {
	static int flag = 1;
	if (flag) {
		program_root = root;
		flag = 0;
	}
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
				fprintf(stderr, "Error line %d: variable %s defined multiple times.\n", root->lineno, root->ident);
				goto free_program_after_error;
			}
			root->offset = tmp_offset;
			root->type = current_node_type;
			if (root->type == TYPE_VOID) {
				fprintf(stderr, "Error line %d: void is not an allowed variable type.\n", root->lineno);
				goto free_program_after_error;
			}
			root->global_decl = is_global;
		} else if (is_fun_decl) {
			if (strcmp(root->ident, "main") == 0 && current_node_type != TYPE_VOID) {
				fprintf(stderr, "Error line %d: main should be void type.\n", root->lineno);
				goto free_program_after_error;
			}
			root->type = current_node_type;
			root->global_decl = is_global;
		} else {
			node_t tmp_node = (node_t) get_decl_node(root->ident);
			// Si la variable n'a pas ete declaree
			if (tmp_node == NULL) {
				fprintf(stderr, "Error line %d: variable '%s' undeclared.\n", root->lineno, root->ident);
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
		root->offset = add_string(root->str);
		break;

	case NODE_INTVAL:
		if (is_rval_in_decl) {
			lval->value = root->value;
		}
		break;

	case NODE_BOOLVAL:
		if (is_rval_in_decl) {
			if (current_node_type != TYPE_BOOL) {
				printf("Error line %d: variable %s of type TYPE_BOOL is initialized with an expression of type %s.\n", lval->lineno, lval->ident, node_type2string(lval->type));
				goto free_program_after_error;
			}
			lval->value = root->value;
		}
		break;

	case NODE_PRINT:
		analyse_passe_1(root->opr[0]);
		break;

	case NODE_IF:
		analyse_passe_1(root->opr[0]);
		if (root->opr[0]->type != TYPE_BOOL) {
			fprintf(stderr, "Error line %d: 'if' condition does not have a boolean type.\n", root->opr[0]->lineno);
			goto free_program_after_error;
		}
		analyse_passe_1(root->opr[1]);
		if (root->nops > 2)
			analyse_passe_1(root->opr[2]);
		break;

	case NODE_FOR:
		analyse_passe_1(root->opr[0]);
		analyse_passe_1(root->opr[1]);
		if ((root->opr[1]->type) != TYPE_BOOL) {
			fprintf(stderr, "Error line %d: 'for' condition does not have a boolean type.\n", root->opr[1]->lineno);
			goto free_program_after_error;
		}
		analyse_passe_1(root->opr[2]);
		analyse_passe_1(root->opr[3]);
		break;

	case NODE_WHILE:
		analyse_passe_1(root->opr[0]);
		if ((root->opr[0]->type) != TYPE_BOOL) {
			fprintf(stderr, "Error line %d: 'while' condition does not have a boolean type.\n", root->opr[0]->lineno);
			goto free_program_after_error;
		}
		analyse_passe_1(root->opr[1]);
		break;

	case NODE_DOWHILE:
		analyse_passe_1(root->opr[0]);
		analyse_passe_1(root->opr[1]);
		if ((root->opr[1]->type) != TYPE_BOOL) {
			fprintf(stderr, "Error line %d: 'while' condition in do-while statement does not have a boolean type.\n", root->opr[1]->lineno);
			goto free_program_after_error;
		}
		break;

	case NODE_AFFECT: case NODE_PLUS: case NODE_MINUS: case NODE_MUL: case NODE_DIV: case NODE_MOD:
	case NODE_BAND: case NODE_BOR: case NODE_BXOR: case NODE_SLL: case NODE_SRL: case NODE_SRA:
	case NODE_EQ: case NODE_NE: case NODE_LT: case NODE_GT: case NODE_LE: case NODE_GE:
	case NODE_AND: case NODE_OR:
		analyse_passe_1(root->opr[0]);
		analyse_passe_1(root->opr[1]);
		root->type = type_op_binaire(root->nature, root->opr[0], root->opr[1]);
		break;

	case NODE_UMINUS: case NODE_BNOT: case NODE_NOT:
		analyse_passe_1(root->opr[0]);
		root->type = type_op_unaire(root->nature, root->opr[0]);
		// type_op_unaire(root->opr[0]->nature, root);
		// root->type = type_op_unaire(root->nature, root);
		break;

	default:
		break;
	}
	return;

free_program_after_error:
	free_global_strings();
	free_nodes(program_root);
	yylex_destroy();
	exit(1);
}
