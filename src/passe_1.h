
#ifndef _PASSE_1_
#define _PASSE_1_

#include "defs.h"


void analyse_passe_1(node_t root);
void verify_vardecls(node_t node);
void verify_decls_list(node_t node);
void verify_decls(node_t node);
void verify_decl_list(node_t node);
void verify_decl(node_t node);
void verify_main(node_t node);
void verify_type(node_t node);
void verify_ident(node_t node);
void verify_block(node_t node);
void verify_insts(node_t node);
void verify_insts_list(node_t node);
void verify_inst(node_t node);
void verify_printparam_list(node_t node);
void verify_printparam(node_t node);
void verify_exp(node_t node);
node_type type_op_unaire(node_t node);
node_type type_op_binaire(node_t node);
#endif

