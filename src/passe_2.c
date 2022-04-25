
// #include <stdio.h>

// #include "defs.h"
// #include "passe_2.h"

// void gen_code_passe_2(node_t root) {

// }

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "defs.h"
#include "passe_2.h"
#include "miniccutils.h"

extern bool is_global;

extern int trace_level;


// void analyse_passe_1(node_t root) {

// }

void gen_code_passe_2(node_t root) {
	printf("------------------------------------------------start passe_2----------------------------------------------\n");
	assert(root->nature == NODE_PROGRAM);

	inst_create_data_sec();

	is_global = 1;

	gen_vardecls(root->opr[0]);
	printf("return to analyse_passe_1\n");

	for (int i = 0; i < get_global_strings_number(); i++)
			inst_create_asciiz(NULL, get_global_string(i));

	inst_create_text_sec();
	inst_create_label_str("main");

	gen_main(root->opr[1]);
	//inst_create_comment("exit");

	inst_create_ori($v0, $zero, EXIT_SYSCALL);
	inst_create_syscall();
}

void gen_vardecls(node_t node){
	printf("start gen_vardecls\n");
	 assert(node == NULL || node->nature == NODE_LIST || node->nature == NODE_DECLS);

	if(node == NULL){
		return;
	}

	switch(node->nature){
		case NODE_LIST:
			gen_decls_list(node->opr[0]);
			printf("return to gen_vardecls\n");
			gen_decls(node->opr[1]);
			printf("return to gen_vardecls\n");break;
		case NODE_DECLS:
			gen_type(node->opr[0]);
			printf("return to gen_vardecls\n");
			gen_decl_list(node->opr[1]);
			printf("return to gen_vardecls\n");break;
	}
	return;
}

void gen_decls_list(node_t node){
	printf("start gen_decls_list\n");
	assert(node->nature ==NODE_LIST || node->nature ==NODE_DECLS);

	switch(node->nature){
		case NODE_LIST:
			gen_decls_list(node->opr[0]);
			printf("return to gen_decls_list\n");
			gen_decls(node->opr[1]);
			printf("return to gen_decls_list\n");break;

		case NODE_DECLS:
			gen_type(node->opr[0]);
			printf("return to gen_decls_list\n");

			gen_decl_list(node->opr[1]);
			printf("return to gen_decls_list\n");break;
	
	}
	return;
}

void gen_decls(node_t node){
	printf("start gen_decls\n");	
	assert(node->nature ==NODE_DECLS);

	switch(node->nature){
		case NODE_DECLS:
			gen_type(node->opr[0]);
			printf("return to gen_decls\n");

			gen_decl_list(node->opr[1]);
			printf("return to gen_decls\n");break;

	} 
	return;
}

void gen_decl_list(node_t node){
	printf("start gen_decl_list\n");	
	assert(node->nature ==NODE_LIST || node->nature ==NODE_DECL); 

	switch(node->nature){
		case NODE_LIST:
			gen_decl_list(node->opr[0]);
			printf("return to gen_decl_list\n");

			gen_decl(node->opr[1]);
			printf("return to gen_decl_list\n");break;

		case NODE_DECL:
			if(node->opr[1] != NULL && is_global == 1)
				inst_create_word(node->opr[0]->ident, node->opr[1]->value);
			else if (is_global == 1)
				inst_create_word(node->opr[0]->ident, 0);//if there is no NODE_INTVAL

			gen_ident(node->opr[0]);
			printf("return to gen_decl_list\n");

			gen_exp(node->opr[1]);
			printf("return to gen_decl_list\n");

			push_temporary(get_current_reg());
			release_reg();break;

	}
	return;
}

void gen_decl(node_t node){
	printf("start gen_decl\n");	
	
	assert(node->nature ==NODE_DECL); 

	
	switch(node->nature){
		case NODE_DECL:
			gen_ident(node->opr[0]);
			printf("return to gen_decl\n");

			gen_exp(node->opr[1]);
			printf("return to gen_decl\n");

			// push_temporary(get_current_reg()-1);
			// release_reg();break;

	}

	return;
}

void gen_main(node_t node){
	printf("start gen_main\n");	
	assert(node->nature == NODE_FUNC);

	is_global = 0;// in main , all variable are not global var.

	//inst_create_or($fp, $zero, $sp);
	set_temporary_start_offset(node->offset);
	inst_create_stack_allocation();
	// printf("%d\n",get_temporary_max_offset());
	//	printf("%d\n",node->offset);


	switch(node->nature){
		case NODE_FUNC:
			gen_type(node->opr[0]);
			printf("return to gen_main\n");
			gen_ident(node->opr[1]);
			printf("return to gen_main\n");

			gen_block(node->opr[2]);	
			printf("return to gen_main\n");break;

	}

	// printf("%d\n",node->offset);


	int32_t stack_size = get_temporary_max_offset();
	printf("%d\n",get_temporary_max_offset());

	inst_create_stack_deallocation(stack_size);

	return;
}

void gen_type(node_t node){
	printf("start gen_type\n");	

	assert(node->nature ==NODE_TYPE);

	return;
 }

void gen_ident(node_t node){
	printf("start gen_ident\n");	

	assert(node->nature ==NODE_IDENT); 


	return;
}

void gen_block(node_t node){
	printf("start gen_block\n");	

	assert(node->nature ==NODE_BLOCK);

	switch(node->nature){
		case NODE_BLOCK:
			gen_vardecls(node->opr[0]);
			printf("return to gen_block\n");

			gen_insts(node->opr[1]);
			printf("return to gen_block\n");break;

	}

	return;
}

void gen_insts(node_t node){
	printf("start gen_insts\n");	

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
			gen_insts_list(node->opr[0]);printf("return to gen_insts\n");
			gen_inst(node->opr[1]);printf("return to gen_insts\n");break;
		case NODE_IF:
			gen_exp(node->opr[0]);printf("return to gen_insts\n");
			gen_inst(node->opr[1]);printf("return to gen_insts\n");
			if(node->opr[2] != NULL){
				gen_inst(node->opr[2]);printf("return to gen_insts\n");
			}	
			break;
		case NODE_WHILE:
			gen_exp(node->opr[0]);printf("return to gen_insts\n");
			gen_inst(node->opr[1]);printf("return to gen_insts\n");break;
		case NODE_DOWHILE:
			gen_inst(node->opr[0]);printf("return to gen_insts\n");
			gen_exp(node->opr[1]);printf("return to gen_insts\n");break;
		case NODE_FOR:
			gen_exp(node->opr[0]);printf("return to gen_insts\n");
			gen_exp(node->opr[1]);printf("return to gen_insts\n");
			gen_exp(node->opr[2]);printf("return to gen_insts\n");
			gen_inst(node->opr[3]);printf("return to gen_insts\n");break;
		case NODE_PRINT:
			gen_printparam_list(node->opr[0]);printf("return to gen_insts\n");break;
		case NODE_BLOCK:
			gen_vardecls(node->opr[0]);printf("return to gen_insts\n");
			gen_insts(node->opr[1]);printf("return to gen_insts\n");break;
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
			gen_exp(node->opr[0]);printf("return to gen_insts\n");
			gen_exp(node->opr[1]);printf("return to gen_insts\n");
			//node->type = type_op_binaire(node->nature, node->opr[0], node->opr[1]);
			break;
		case NODE_NOT:
		case NODE_BNOT:
		case NODE_UMINUS:
			gen_exp(node->opr[0]);printf("return to gen_insts\n");
			//node->type = type_op_unaire(node->nature, node->opr[0]);
			break;

		case NODE_AFFECT:
			gen_ident(node->opr[0]);printf("return to gen_insts\n");
			gen_exp(node->opr[1]);printf("return to gen_insts\n");
			push_temporary(get_current_reg());
			release_reg();
			//node->type = type_op_binaire(node->nature, node->opr[0], node->opr[1]);
			break;
		case NODE_INTVAL:					
			reg_for_intval(node);
			break;
		case NODE_BOOLVAL:
			return;
		case NODE_IDENT:	
			gen_ident(node);
			return;
	}
	return;
}

void gen_insts_list(node_t node){
	printf("start gen_insts_list\n");	

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
				|| node->nature == NODE_NOT
				|| node->nature == NODE_BNOT
				|| node->nature == NODE_AFFECT
				|| node->nature == NODE_INTVAL
				|| node->nature == NODE_BOOLVAL
				|| node->nature == NODE_IDENT	//ident
			);  
	switch(node->nature){
		case NODE_LIST:
			gen_insts_list(node->opr[0]);printf("return to gen_insts_list\n");
			gen_inst(node->opr[1]);printf("return to gen_insts_list\n");break;
		case NODE_IF:
			gen_exp(node->opr[0]);printf("return to gen_insts_list\n");
			gen_inst(node->opr[1]);printf("return to gen_insts_list\n");
			if(node->opr[2] != NULL){
				gen_inst(node->opr[2]);printf("return to gen_insts_list");
			}	
			break;
		case NODE_WHILE:
			gen_exp(node->opr[0]);printf("return to gen_insts_list\n");
			gen_inst(node->opr[1]);printf("return to gen_insts_list\n");break;
		case NODE_DOWHILE:
			gen_inst(node->opr[0]);printf("return to gen_insts_list\n");
			gen_exp(node->opr[1]);printf("return to gen_insts_list\n");break;
		case NODE_FOR:
			gen_exp(node->opr[0]);printf("return to gen_insts_list\n");
			gen_exp(node->opr[1]);printf("return to gen_insts_list\n");
			gen_exp(node->opr[2]);printf("return to gen_insts_list\n");
			gen_inst(node->opr[3]);printf("return to gen_insts_list\n");break;
		case NODE_PRINT:
			gen_printparam_list(node->opr[0]);printf("return to gen_insts_list\n");break;
		case NODE_BLOCK:
			gen_vardecls(node->opr[0]);printf("return to gen_insts_list\n");
			gen_insts(node->opr[1]);printf("return to gen_insts_list\n");break;
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
			gen_exp(node->opr[0]);printf("return to gen_insts_list\n");
			gen_exp(node->opr[1]);printf("return to gen_insts_list\n");
			reg_for_exp_binaire(node);break;

		case NODE_UMINUS:
		case NODE_NOT:
		case NODE_BNOT:
			gen_exp(node->opr[0]);printf("return to gen_insts_list\n");break;
		case NODE_AFFECT:
			gen_ident(node->opr[0]);printf("return to gen_insts_list\n");
			gen_exp(node->opr[1]);printf("return to gen_insts_list\n");
			push_temporary(get_current_reg());
			release_reg();break;
		case NODE_INTVAL:
			reg_for_intval(node);
			break;
		case NODE_BOOLVAL:
			return;
		case NODE_IDENT:
			gen_ident(node);
			return;
	}
	return;
}

void gen_inst(node_t node){
	printf("start gen_inst\n");	

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
				|| node->nature == NODE_NOT
				|| node->nature == NODE_BNOT
				|| node->nature == NODE_AFFECT
				|| node->nature == NODE_INTVAL
				|| node->nature == NODE_BOOLVAL
				|| node->nature == NODE_IDENT	//ident
			);  
	switch(node->nature){
		case NODE_IF:
			gen_exp(node->opr[0]);printf("return to gen_inst\n");
			gen_inst(node->opr[1]);printf("return to gen_inst\n");
			if(node->opr[2] != NULL){
				gen_inst(node->opr[2]);printf("return to gen_inst\n");
			}	
			break;
		case NODE_WHILE:
			gen_exp(node->opr[0]);printf("return to gen_inst\n");
			gen_inst(node->opr[1]);printf("return to gen_inst\n");break;
		case NODE_DOWHILE:
			gen_inst(node->opr[0]);printf("return to gen_inst\n");
			gen_exp(node->opr[1]);printf("return to gen_inst\n");break;
		case NODE_FOR:
			gen_exp(node->opr[0]);printf("return to gen_inst\n");
			gen_exp(node->opr[1]);printf("return to gen_inst\n");
			gen_exp(node->opr[2]);printf("return to gen_inst\n");
			gen_inst(node->opr[3]);printf("return to gen_inst\n");break;
		case NODE_PRINT:
			gen_printparam_list(node->opr[0]);printf("return to gen_inst\n");break;
		case NODE_BLOCK:
			gen_vardecls(node->opr[0]);printf("return to gen_inst\n");
			gen_insts(node->opr[1]);printf("return to gen_inst\n");break;
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
			gen_exp(node->opr[0]);printf("return to gen_inst\n");
			gen_exp(node->opr[1]);printf("return to gen_inst\n");
			reg_for_exp_binaire(node);break;

		case NODE_UMINUS:
		case NODE_NOT:
		case NODE_BNOT:
			gen_exp(node->opr[0]);printf("return to gen_inst\n");break;
		case NODE_AFFECT:
			gen_ident(node->opr[0]);printf("return to gen_inst\n");
			gen_exp(node->opr[1]);printf("return to gen_inst\n");
			push_temporary(get_current_reg());
			release_reg();break;
		case NODE_INTVAL:
			reg_for_intval(node);
			break;
		case NODE_BOOLVAL:
			return;
		case NODE_IDENT:
			gen_ident(node);
			return;
	}
	return;
}

void gen_printparam_list(node_t node){
	printf("start gen_printparam_list\n");	

	assert(node->nature ==NODE_LIST || node->nature ==NODE_STRINGVAL || node->nature ==NODE_IDENT); 

	switch(node->nature){
		case NODE_LIST:
			gen_printparam_list(node->opr[0]);printf("return to gen_printparam_list\n");
			gen_printparam(node->opr[1]);printf("return to gen_printparam_list\n");break;
		case NODE_STRINGVAL:
			//node->offset = add_string(node->str);
			return;
		case NODE_IDENT:
			gen_ident(node);
			return;
	}
	return;
}

void gen_printparam(node_t node){
	printf("start gen_printparam\n");	

	assert(node->nature ==NODE_STRINGVAL || node->nature ==NODE_IDENT); 

	switch(node->nature){
		case NODE_STRINGVAL:
			//node->offset = add_string(node->str);
			return;
		case NODE_IDENT:
			gen_ident(node);
			return;
	}
	return;
}

void gen_exp(node_t node){
	printf("start gen_exp\n");	

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
			gen_exp(node->opr[0]);printf("return to gen_exp\n");
			gen_exp(node->opr[1]);printf("return to gen_exp\n");
			reg_for_exp_binaire(node);break;

		case NODE_UMINUS:
		case NODE_NOT:
		case NODE_BNOT:
			gen_exp(node->opr[0]);printf("return to gen_exp\n");break;
		case NODE_AFFECT:
			gen_ident(node->opr[0]);printf("return to gen_exp\n");
			gen_exp(node->opr[1]);printf("return to gen_exp\n");
			push_temporary(get_current_reg());
			release_reg();break;
		case NODE_INTVAL:
			reg_for_intval(node);
			break;
		case NODE_BOOLVAL:
			return;
		case NODE_IDENT:
			gen_ident(node);
			return;
	}

	return;
}


void reg_for_intval(node_t node){
	int reg;
	reg =get_current_reg();

	// printf("%d-----------------\n",reg_available());
	if(reg_available()){
		allocate_reg();
	}
	else
		reg = get_restore_reg();
	if(node->global_decl == 0)
		inst_create_ori(reg, $zero, node->value);
	return;
}

void reg_for_exp_binaire(node_t node){
	int reg1,reg2;

	//for reg1
	reg1 =get_current_reg();
	//for reg2
	release_reg();

	reg2 =get_current_reg();

	switch(node->nature){
		case NODE_PLUS:
			inst_create_addu(reg1,reg1,reg2);
			break;
		case NODE_MINUS:
			inst_create_subu(reg1,reg1,reg2);break;
		case NODE_MUL:
			inst_create_mult(reg1,reg2);
			inst_create_mflo(reg1);break;
		case NODE_DIV:
			inst_create_div(reg1,reg2);
			inst_create_teq(reg2, $zero);
			inst_create_mflo(reg1);break;
			break;
		case NODE_MOD:
			inst_create_div(reg1, reg2);
			inst_create_teq(reg2, $zero);
			inst_create_mfhi(reg1);break;
			break;
		case NODE_LT:
			inst_create_slt(reg1,reg1,reg2);break;
		case NODE_GT:
			inst_create_slt(reg1,reg2,reg1);break;
		case NODE_LE:
			inst_create_slt(reg1,reg2,reg1);
			inst_create_xori(reg1,reg1,1);
			break;
		case NODE_GE:
			inst_create_slt(reg1,reg1,reg2);
			inst_create_xori(reg1,reg1,1);
			break;
		case NODE_EQ:
			inst_create_xor(reg1,reg1,reg2);
			inst_create_sltiu(reg1,reg2,1);
			break;
		case NODE_NE:
			inst_create_xor(reg1,reg1,reg2);
			inst_create_sltu(reg1,$zero,reg1);break;
		case NODE_AND:
			inst_create_and(reg1,reg1,reg2);break;
		case NODE_OR:
			inst_create_or(reg1,reg1,reg2);;break;
		case NODE_BAND:
			inst_create_and(reg1,reg1,reg2);break;
		case NODE_BOR:
			inst_create_or(reg1,reg1,reg2);break;
		case NODE_BXOR:
			inst_create_xor(reg1,reg1,reg2);break;
		case NODE_SRL:
			inst_create_srlv(reg1,reg1,reg2);break;
		case NODE_SRA:
			inst_create_srav(reg1,reg1,reg2);break;
		case NODE_SLL:
			inst_create_sllv(reg1,reg1,reg2);break;

	}
	//push_temporary(reg1);
	//release_reg(reg1);
	//release_reg(reg2);

	return;
}