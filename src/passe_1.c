
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "defs.h"
#include "passe_1.h"
#include "miniccutils.h"

extern int trace_level;
extern int yylineno;

// void analyse_passe_1(node_t root) {

// }
bool is_global;
node_type current_type;

void analyse_passe_1(node_t root) {
	printf("-------------------------------------------------------start passe_1!--------------------------------\n");
	assert(root->nature == NODE_PROGRAM);
	is_global = true; 

	push_global_context();

	verify_vardecls(root->opr[0]);
	printf("return to analyse_passe_1\n");

	is_global = false; 

	verify_main(root->opr[1]);

	pop_context();
}

void verify_vardecls(node_t node){
	printf("start verify_vardecls\n");
	assert(node == NULL || node->nature == NODE_LIST || node->nature == NODE_DECLS);


	if(node == NULL){
		return;
	}

	node->global_decl = is_global;
	//printf("%d\n",is_global);
	switch(node->nature){
		case NODE_LIST:
			verify_decls_list(node->opr[0]);
			printf("return to verify_vardecls\n");
			verify_decls(node->opr[1]);
			printf("return to verify_vardecls\n");break;
		case NODE_DECLS:
			verify_type(node->opr[0]);
			printf("return to verify_vardecls\n");
			verify_decl_list(node->opr[1]);
			printf("return to verify_vardecls\n");break;
	}
	return;
}

void verify_decls_list(node_t node){
	printf("start verify_decls_list\n");
	assert(node->nature ==NODE_LIST || node->nature ==NODE_DECLS);


	switch(node->nature){
		case NODE_LIST:
			verify_decls_list(node->opr[0]);
			printf("return to verify_decls_list\n");
			verify_decls(node->opr[1]);
			printf("return to verify_decls_list\n");break;

		case NODE_DECLS:
			verify_type(node->opr[0]);
			printf("return to verify_decls_list\n");

			verify_decl_list(node->opr[1]);
			printf("return to verify_decls_list\n");break;
	
	}
	return;
}

void verify_decls(node_t node){
	printf("start verify_decls\n");	
	assert(node->nature ==NODE_DECLS); 

	switch(node->nature){
		case NODE_DECLS:
			verify_type(node->opr[0]);
			printf("return to verify_decls\n");

			verify_decl_list(node->opr[1]);
			printf("return to verify_decls\n");break;

	} 
	return;
}

void verify_decl_list(node_t node){
	printf("start verify_decl_list\n");	
	assert(node->nature ==NODE_LIST || node->nature ==NODE_DECL); 

	switch(node->nature){
		case NODE_LIST:
			verify_decl_list(node->opr[0]);
			printf("return to verify_decl_list\n");

			verify_decl(node->opr[1]);
			printf("return to verify_decl_list\n");break;

		case NODE_DECL:
			verify_ident(node->opr[0]);
			printf("return to verify_decl_list\n");

			verify_exp(node->opr[1]);
			printf("return to verify_decl_list\n");break;

	}
	return;
}

void verify_decl(node_t node){
	printf("start verify_decl\n");	
	
	assert(node->nature ==NODE_DECL); 

	
	switch(node->nature){
		case NODE_DECL:
			verify_ident(node->opr[0]);
			printf("return to verify_decl\n");

			verify_exp(node->opr[1]);
			printf("return to verify_decl\n");break;

	}
	return;
}

void verify_main(node_t node){
	printf("start verify_main\n");	
	assert(node->nature == NODE_FUNC);

	

	switch(node->nature){ 
		case NODE_FUNC:
			verify_type(node->opr[0]);
			printf("return to verify_main\n");

			verify_ident(node->opr[1]);
			printf("return to verify_main\n");

			//check if the func is named 'main' 
			if(strcmp(node->opr[1]->ident,"main")!=0){
				printf("ERROR line %d: ident for fonction should be 'main' \n",yylineno);
				exit(0);
			}

			reset_env_current_offset();

			verify_block(node->opr[2]);	

			printf("return to verify_main\n");break;

	}
	
	node->offset = get_env_current_offset();
	printf("%d\n",node->offset);



	return;
}

void verify_type(node_t node){
	printf("start verify_type\n");	
	assert(node->nature ==NODE_TYPE);

	current_type = node->type;

	if(node->type != TYPE_BOOL && node->type != TYPE_INT && node->type != TYPE_VOID)
		fprintf(stderr, "Error type %d\n", yylineno);

	return;
 }

void verify_ident(node_t node){
	printf("start verify_ident\n");	

	assert(node->nature ==NODE_IDENT);

	//test ident main
	if(strcmp(node->ident,"main")==0){
		return;
	}// except "main"


	//test global var and update global_decl
	if(is_global == true)
		node->global_decl = true;
	
	else
		node->global_decl = false;
	
	//check if there is a decl before
	node_t n = (node_t)get_decl_node(node->ident);
	//if not, add element
	if(n == NULL){
		int off =env_add_element(node->ident,node);
		
		if(off>= 0){
			printf("add element:%s, and its offset is %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",node->ident,off);
			node->offset = off;
		}

		node->type = current_type;
	}
	else{
		node->decl_node =  n;
		node->type = n->type;
		node->offset= n->offset;
		printf("element %s deja exist\n",node->ident);
	}
	return;
}

void verify_block(node_t node){
	printf("start verify_block\n");	

	assert(node->nature ==NODE_BLOCK);

	push_context(); 

	switch(node->nature){
		case NODE_BLOCK:
			verify_vardecls(node->opr[0]);
			printf("return to verify_block\n");

			verify_insts(node->opr[1]);
			printf("return to verify_block\n");break;

	}

	pop_context();

	return;
}

void verify_insts(node_t node){
	printf("start verify_insts\n");	

	assert(		node == NULL 
				|| node->nature == NODE_LIST 	
				|| node->nature == NODE_IF
				|| node->nature == NODE_WHILE
				|| node->nature == NODE_DOWHILE
				|| node->nature == NODE_FOR
				|| node->nature == NODE_PRINT
				|| node->nature == NODE_BLOCK	//block
				|| node->nature == NODE_PLUS	//exp
				|| node->nature == NODE_MINUS
				|| node->nature == NODE_MUL
				|| node->nature == NODE_DIV
				|| node->nature == NODE_MOD
				|| node->nature == NODE_UMINUS
				|| node->nature == NODE_LT
				|| node->nature == NODE_GT
				|| node->nature == NODE_LE
				|| node->nature == NODE_GE 
				|| node->nature == NODE_EQ
				|| node->nature == NODE_NE
				|| node->nature == NODE_AND
				|| node->nature == NODE_OR 
				|| node->nature == NODE_BAND
				|| node->nature == NODE_BOR
				|| node->nature == NODE_BXOR
				|| node->nature == NODE_SRL
				|| node->nature == NODE_SRA
				|| node->nature == NODE_SLL
				|| node->nature == NODE_NOT
				|| node->nature == NODE_BNOT
				|| node->nature == NODE_AFFECT
				|| node->nature == NODE_INTVAL
				|| node->nature == NODE_BOOLVAL
				|| node->nature == NODE_IDENT	//ident
			); 
	
	if(node == NULL){
		return;
	}

	switch(node->nature){
		case NODE_LIST:
			verify_insts_list(node->opr[0]);printf("return to verify_insts\n");
			verify_inst(node->opr[1]);printf("return to verify_insts\n");break;
		case NODE_IF:
			verify_exp(node->opr[0]);printf("return to verify_insts\n");
			verify_inst(node->opr[1]);printf("return to verify_insts\n");
			if(node->opr[2] != NULL){
				verify_inst(node->opr[2]);printf("return to verify_insts\n");
			}	
			break;
		case NODE_WHILE:
			verify_exp(node->opr[0]);printf("return to verify_insts\n");
			verify_inst(node->opr[1]);printf("return to verify_insts\n");break;
		case NODE_DOWHILE:
			verify_inst(node->opr[0]);printf("return to verify_insts\n");
			verify_exp(node->opr[1]);printf("return to verify_insts\n");break;
		case NODE_FOR:
			verify_exp(node->opr[0]);printf("return to verify_insts\n");
			verify_exp(node->opr[1]);printf("return to verify_insts\n");
			verify_exp(node->opr[2]);printf("return to verify_insts\n");
			verify_inst(node->opr[3]);printf("return to verify_insts\n");break;
		case NODE_PRINT:
			verify_printparam_list(node->opr[0]);printf("return to verify_insts\n");break;
		case NODE_BLOCK:
			verify_vardecls(node->opr[0]);printf("return to verify_insts\n");
			verify_insts(node->opr[1]);printf("return to verify_insts\n");break;
		case NODE_PLUS:
		case NODE_MINUS:
		case NODE_MUL:
		case NODE_DIV:
		case NODE_MOD:
		case NODE_LT:
		case NODE_GT:
		case NODE_LE:
		case NODE_GE:
		case NODE_EQ:
		case NODE_NE:
		case NODE_AND:
		case NODE_OR:
		case NODE_BAND:
		case NODE_BOR:
		case NODE_BXOR:
		case NODE_SRL:
		case NODE_SRA:
		case NODE_SLL:
			verify_exp(node->opr[0]);printf("return to verify_insts\n");
			verify_exp(node->opr[1]);printf("return to verify_insts\n");
			node->type = type_op_binaire(node);
			break;
		case NODE_NOT:
		case NODE_BNOT:
		case NODE_UMINUS:
			node->type = type_op_unaire(node);
			verify_exp(node->opr[0]);printf("return to verify_insts\n");
			break;

		case NODE_AFFECT:
			verify_ident(node->opr[0]);printf("return to verify_insts\n");
			verify_exp(node->opr[1]);printf("return to verify_insts\n");
			node->type = node->opr[1]->type;
			break;
		case NODE_INTVAL:
			node->global_decl = is_global;

			return;
		case NODE_BOOLVAL:
			return;
		case NODE_IDENT:	
			verify_ident(node);
			return;
	}
	return;
}

void verify_insts_list(node_t node){
	printf("start verify_insts_list\n");	

	assert(		node->nature == NODE_LIST 	
				|| node->nature == NODE_IF
				|| node->nature == NODE_WHILE
				|| node->nature == NODE_DOWHILE
				|| node->nature == NODE_FOR
				|| node->nature == NODE_PRINT
				|| node->nature == NODE_BLOCK	//block
				|| node->nature == NODE_PLUS	//exp
				|| node->nature == NODE_MINUS
				|| node->nature == NODE_MUL
				|| node->nature == NODE_DIV
				|| node->nature == NODE_MOD
				|| node->nature == NODE_UMINUS
				|| node->nature == NODE_LT
				|| node->nature == NODE_GT
				|| node->nature == NODE_LE
				|| node->nature == NODE_GE 
				|| node->nature == NODE_EQ
				|| node->nature == NODE_NE
				|| node->nature == NODE_AND
				|| node->nature == NODE_OR 
				|| node->nature == NODE_BAND
				|| node->nature == NODE_BOR
				|| node->nature == NODE_BXOR
				|| node->nature == NODE_SRL
				|| node->nature == NODE_SRA
				|| node->nature == NODE_SLL
				|| node->nature == NODE_NOT
				|| node->nature == NODE_BNOT
				|| node->nature == NODE_AFFECT
				|| node->nature == NODE_INTVAL
				|| node->nature == NODE_BOOLVAL
				|| node->nature == NODE_IDENT	//ident
			);  
	switch(node->nature){
		case NODE_LIST:
			verify_insts_list(node->opr[0]);printf("return to verify_insts_list\n");
			verify_inst(node->opr[1]);printf("return to verify_insts_list\n");break;
		case NODE_IF:
			verify_exp(node->opr[0]);printf("return to verify_insts_list\n");
			verify_inst(node->opr[1]);printf("return to verify_insts_list\n");
			if(node->opr[2] != NULL){
				verify_inst(node->opr[2]);printf("return to verify_insts_list");
			}	
			break;
		case NODE_WHILE:
			verify_exp(node->opr[0]);printf("return to verify_insts_list\n");
			verify_inst(node->opr[1]);printf("return to verify_insts_list\n");break;
		case NODE_DOWHILE:
			verify_inst(node->opr[0]);printf("return to verify_insts_list\n");
			verify_exp(node->opr[1]);printf("return to verify_insts_list\n");break;
		case NODE_FOR:
			verify_exp(node->opr[0]);printf("return to verify_insts_list\n");
			verify_exp(node->opr[1]);printf("return to verify_insts_list\n");
			verify_exp(node->opr[2]);printf("return to verify_insts_list\n");
			verify_inst(node->opr[3]);printf("return to verify_insts_list\n");break;
		case NODE_PRINT:
			verify_printparam_list(node->opr[0]);printf("return to verify_insts_list\n");break;
		case NODE_BLOCK:
			verify_vardecls(node->opr[0]);printf("return to verify_insts_list\n");
			verify_insts(node->opr[1]);printf("return to verify_insts_list\n");break;
		case NODE_PLUS:
		case NODE_MINUS:
		case NODE_MUL:
		case NODE_DIV:
		case NODE_MOD:
		case NODE_LT:
		case NODE_GT:
		case NODE_LE:
		case NODE_GE:
		case NODE_EQ:
		case NODE_NE:
		case NODE_AND:
		case NODE_OR:
		case NODE_BAND:
		case NODE_BOR:
		case NODE_BXOR:
		case NODE_SRL:
		case NODE_SRA:
		case NODE_SLL:
			verify_exp(node->opr[0]);printf("return to verify_insts_list\n");
			verify_exp(node->opr[1]);printf("return to verify_insts_list\n");
			node->type = type_op_binaire(node);
			break;
		case NODE_UMINUS:
		case NODE_NOT:
		case NODE_BNOT:
			node->type = type_op_unaire(node);
			verify_exp(node->opr[0]);printf("return to verify_insts_list\n");break;
		case NODE_AFFECT:
			verify_ident(node->opr[0]);printf("return to verify_insts_list\n");
			verify_exp(node->opr[1]);printf("return to verify_insts_list\n");
			node->type = node->opr[1]->type;break;
		case NODE_INTVAL:

			node->global_decl = is_global;

			return;
		case NODE_BOOLVAL:
			return;
		case NODE_IDENT:
			verify_ident(node);
			return;
	}
	return;
}

void verify_inst(node_t node){
	printf("start verify_inst\n");	

	assert(		node->nature == NODE_IF
				|| node->nature == NODE_IF
				|| node->nature == NODE_WHILE
				|| node->nature == NODE_DOWHILE
				|| node->nature == NODE_FOR
				|| node->nature == NODE_PRINT
				|| node->nature == NODE_BLOCK	//block
				|| node->nature == NODE_PLUS	//exp
				|| node->nature == NODE_MINUS
				|| node->nature == NODE_MUL
				|| node->nature == NODE_DIV
				|| node->nature == NODE_MOD
				|| node->nature == NODE_UMINUS
				|| node->nature == NODE_LT
				|| node->nature == NODE_GT
				|| node->nature == NODE_LE
				|| node->nature == NODE_GE 
				|| node->nature == NODE_EQ
				|| node->nature == NODE_NE
				|| node->nature == NODE_AND
				|| node->nature == NODE_OR 
				|| node->nature == NODE_BAND
				|| node->nature == NODE_BOR
				|| node->nature == NODE_BXOR
				|| node->nature == NODE_SRL
				|| node->nature == NODE_SRA
				|| node->nature == NODE_SLL
				|| node->nature == NODE_NOT
				|| node->nature == NODE_BNOT
				|| node->nature == NODE_AFFECT
				|| node->nature == NODE_INTVAL
				|| node->nature == NODE_BOOLVAL
				|| node->nature == NODE_IDENT	//ident
			);  
	switch(node->nature){
		case NODE_IF:
			verify_exp(node->opr[0]);printf("return to verify_inst\n");
			verify_inst(node->opr[1]);printf("return to verify_inst\n");
			if(node->opr[2] != NULL){
				verify_inst(node->opr[2]);printf("return to verify_inst\n");
			}	
			break;
		case NODE_WHILE:
			verify_exp(node->opr[0]);printf("return to verify_inst\n");
			verify_inst(node->opr[1]);printf("return to verify_inst\n");break;
		case NODE_DOWHILE:
			verify_inst(node->opr[0]);printf("return to verify_inst\n");
			verify_exp(node->opr[1]);printf("return to verify_inst\n");break;
		case NODE_FOR:
			verify_exp(node->opr[0]);printf("return to verify_inst\n");
			verify_exp(node->opr[1]);printf("return to verify_inst\n");
			verify_exp(node->opr[2]);printf("return to verify_inst\n");
			verify_inst(node->opr[3]);printf("return to verify_inst\n");break;
		case NODE_PRINT:
			verify_printparam_list(node->opr[0]);printf("return to verify_inst\n");break;
		case NODE_BLOCK:
			verify_vardecls(node->opr[0]);printf("return to verify_inst\n");
			verify_insts(node->opr[1]);printf("return to verify_inst\n");break;
		case NODE_PLUS:
		case NODE_MINUS:
		case NODE_MUL:
		case NODE_DIV:
		case NODE_MOD:
		case NODE_LT:
		case NODE_GT:
		case NODE_LE:
		case NODE_GE:
		case NODE_EQ:
		case NODE_NE:
		case NODE_AND:
		case NODE_OR:
		case NODE_BAND:
		case NODE_BOR:
		case NODE_BXOR:
		case NODE_SRL:
		case NODE_SRA:
		case NODE_SLL:
			verify_exp(node->opr[0]);printf("return to verify_inst\n");
			verify_exp(node->opr[1]);printf("return to verify_inst\n");
			node->type = type_op_binaire(node);
			break;
		case NODE_UMINUS:
		case NODE_NOT:
		case NODE_BNOT:
			node->type = type_op_unaire(node);
			verify_exp(node->opr[0]);printf("return to verify_inst\n");break;
		case NODE_AFFECT:
			verify_ident(node->opr[0]);printf("return to verify_inst\n");
			verify_exp(node->opr[1]);printf("return to verify_inst\n");
			node->type = node->opr[1]->type;break;
		case NODE_INTVAL:
			node->global_decl = is_global;

			return;
		case NODE_BOOLVAL:
			return;
		case NODE_IDENT:
			verify_ident(node);
			return;
	}
	return;
}

void verify_printparam_list(node_t node){
	printf("start verify_printparam_list\n");	

	assert(node->nature ==NODE_LIST || node->nature ==NODE_STRINGVAL || node->nature ==NODE_IDENT); 

	switch(node->nature){
		case NODE_LIST:
			verify_printparam_list(node->opr[0]);printf("return to verify_printparam_list\n");
			verify_printparam(node->opr[1]);printf("return to verify_printparam_list\n");break;
		case NODE_STRINGVAL:
			node->offset = add_string(node->str);
			return;
		case NODE_IDENT:
			verify_ident(node);
			return;
	}
	return;
}

void verify_printparam(node_t node){
	printf("start verify_printparam\n");	

	assert(node->nature ==NODE_STRINGVAL || node->nature ==NODE_IDENT); 

	switch(node->nature){
		case NODE_STRINGVAL:
			node->offset = add_string(node->str);
			return;
		case NODE_IDENT:
			verify_ident(node);
			return;
	}
	return;
}

void verify_exp(node_t node){
	printf("start verify_exp\n");	

	assert(		node == NULL
				||node->nature == NODE_PLUS	//exp
				|| node->nature == NODE_MINUS
				|| node->nature == NODE_MUL
				|| node->nature == NODE_DIV
				|| node->nature == NODE_MOD
				|| node->nature == NODE_UMINUS
				|| node->nature == NODE_LT
				|| node->nature == NODE_GT
				|| node->nature == NODE_LE
				|| node->nature == NODE_GE 
				|| node->nature == NODE_EQ
				|| node->nature == NODE_NE
				|| node->nature == NODE_AND
				|| node->nature == NODE_OR 
				|| node->nature == NODE_BAND
				|| node->nature == NODE_BOR
				|| node->nature == NODE_BXOR
				|| node->nature == NODE_SRL
				|| node->nature == NODE_SRA
				|| node->nature == NODE_SLL
				|| node->nature == NODE_NOT
				|| node->nature == NODE_BNOT
				|| node->nature == NODE_AFFECT
				|| node->nature == NODE_INTVAL
				|| node->nature == NODE_BOOLVAL
				|| node->nature == NODE_IDENT	//ident
			); 

	if(node == NULL){
		return;
	}

	switch(node->nature){
		case NODE_PLUS:
		case NODE_MINUS:
		case NODE_MUL:
		case NODE_DIV:
		case NODE_MOD:
		case NODE_LT:
		case NODE_GT:
		case NODE_LE:
		case NODE_GE:
		case NODE_EQ:
		case NODE_NE:
		case NODE_AND:
		case NODE_OR:
		case NODE_BAND:
		case NODE_BOR:
		case NODE_BXOR:
		case NODE_SRL:
		case NODE_SRA:
		case NODE_SLL:
			
			verify_exp(node->opr[0]);printf("return to verify_exp\n");
			verify_exp(node->opr[1]);printf("return to verify_exp\n");
			node->type = type_op_binaire(node);break;
		case NODE_UMINUS:
		case NODE_NOT:
		case NODE_BNOT:
			node->type = type_op_unaire(node);
			verify_exp(node->opr[0]);printf("return to verify_exp\n");break;
		case NODE_AFFECT:
			verify_ident(node->opr[0]);printf("return to verify_exp\n");
			verify_exp(node->opr[1]);printf("return to verify_exp\n");
			node->type = node->opr[1]->type;break;
		case NODE_INTVAL:
			node->global_decl = is_global;

			return;
		case NODE_BOOLVAL:
			return;
		case NODE_IDENT:
			verify_ident(node);
			return;
	}
	return;
}

node_type type_op_unaire(node_t node) {
	
	if(node->nature == NODE_UMINUS || node->nature == NODE_BNOT)
		return TYPE_INT;
	else if (node->nature == NODE_NOT)
		return TYPE_BOOL;
	else
		printf("unaire Type Error: line %d \n",yylineno);
		exit(0);
}

node_type type_op_binaire(node_t node) {
	if(node->opr[0]->type != node->opr[1]->type){
		printf("binaireType Error : line %d \n",yylineno);
		exit(0);
	}
	else if(node->nature == NODE_PLUS
		||node->nature == NODE_MINUS
		||node->nature == NODE_MUL
		||node->nature == NODE_DIV
		||node->nature == NODE_MOD
		||node->nature == NODE_BAND
		||node->nature == NODE_BOR
		||node->nature == NODE_BXOR
		||node->nature == NODE_SLL
		||node->nature == NODE_SRL
		||node->nature == NODE_SRA
		||node->nature == NODE_AND
		||node->nature == NODE_OR)
		//return int or bool	
		return node->opr[0]->type;
	else if(node->nature == NODE_EQ
		||node->nature == NODE_NE
		||node->nature == NODE_LT
		||node->nature == NODE_GT
		||node->nature == NODE_LE
		||node->nature == NODE_GE)
		//return type bool only
		return TYPE_BOOL;
}