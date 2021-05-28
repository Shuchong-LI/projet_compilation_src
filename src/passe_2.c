#include "miniccutils.h"
#include <stdio.h>

#include "defs.h"
#include "passe_2.h"

void print_handler(node_t root);
void block_allocation(node_t root);
int32_t reg_allocation(int *is_reg_available);
void reg_desallocation(int32_t tmp_reg, int is_reg_available);

void gen_code_passe_2(node_t root) {
	if (root == NULL)
		return;
	
	switch (root->nature) {
	case NODE_PROGRAM:
		create_inst_data_sec();
		gen_code_passe_2(root->opr[0]);
		// Allocation des chaines des caracteres
		for (int i = 0; i < get_global_strings_number(); i++)
			create_inst_asciiz(NULL, get_global_string(i));

		create_inst_text_sec();
		gen_code_passe_2(root->opr[1]);

		create_inst_comment("exit");
		create_inst_ori($v0, $zero, EXIT_SYSCALL); // $v0 <- 10
		create_inst_syscall();
		break;
	
	case NODE_LIST:
		gen_code_passe_2(root->opr[0]);
		gen_code_passe_2(root->opr[1]);
		break;
		
	case NODE_DECLS:
		gen_code_passe_2(root->opr[1]);
		break;

	case NODE_DECL:
		gen_code_passe_2(root->opr[0]);
		break;

	case NODE_IDENT:
		// Première déclaration
		if (root->decl_node == NULL)
			// Variable globale
			if (root->global_decl)
				create_inst_word(root->ident, root->value);
		break;

	case NODE_FUNC: //TODO
		// Allocation pile
		create_inst_or($fp, $zero, $sp);
		set_temporary_start_offset(root->offset);
		create_inst_stack_allocation();
		gen_code_passe_2(root->opr[2]);
		int32_t stack_size = get_temporary_max_offset() + root->offset;
		create_inst_stack_deallocation(stack_size);
		break;
	
	case NODE_BLOCK: //TODO
		// Remplissage pile
		block_allocation(root->opr[0]);
		gen_code_passe_2(root->opr[1]);
		break;

	case NODE_PRINT:
		print_handler(root->opr[0]);
		break;

	case NODE_FOR:
		gen_code_passe_2(root->opr[0]);		// Handle the first instruction
		break;

	case NODE_AFFECT:
		if (!reg_available) {

		}

	default:
		break;
	}
}

void print_handler(node_t root)
{
	if (root->nature == NODE_LIST) {
		print_handler(root->opr[0]);
		print_handler(root->opr[1]);
	} else if (root->nature == NODE_STRINGVAL) {
		create_inst_comment("print");
		create_inst_ori($v0, $zero, PRINT_STRING_SYSCALL);		// $v0 <- 4
		create_inst_lui($a0, DATA_SECTION_BASE_ADDRESS);		// fetch global address
		create_inst_addiu($a0, $a0, root->offset);			// add the offset
		create_inst_syscall();
	} else {
		create_inst_comment("print");
		create_inst_ori($v0, $zero, PRINT_INTEGER_SYSCALL);		// $v0 <- 1
		if (root->global_decl)
			create_inst_lui($a0, DATA_SECTION_BASE_ADDRESS);	// fetch global address
		else
			create_inst_or($a0, $zero, $sp);			// fetch stack address
		create_inst_lw($a0, root->offset, $a0);				// add the offset
		create_inst_syscall();
	}
}

void block_allocation(node_t root)
{
	if (root == NULL)
		return;

	switch (root->nature) {
	case NODE_LIST:
		block_allocation(root->opr[0]);
		block_allocation(root->opr[1]);
		break;
	case NODE_DECLS:
		block_allocation(root->opr[1]);
		break;
	case NODE_DECL:
		int32_t tmp_reg;
		int is_reg_available;
		tmp_reg = reg_allocation(&is_reg_available);

		create_inst_ori(tmp_reg, $zero, root->opr[0]->value);
		create_inst_sw(tmp_reg, root->opr[0]->offset, $sp);

		reg_desallocation(tmp_reg, is_reg_available);
		// TODO : operation
		break;
	}
}

int32_t reg_allocation(int *is_reg_available)
{
	int32_t tmp_reg;
	*is_reg_available = reg_available();
	if (!(*is_reg_available)) {
		tmp_reg = get_restore_reg();
		push_temporary(tmp_reg);
		//fprintf(stderr, "Error : not enough register\n");
	} else {
		allocate_reg();
		tmp_reg = get_current_reg();
	}
	return tmp_reg;
}

void reg_desallocation(int32_t tmp_reg, int is_reg_available)
{
	if (!is_reg_available)
		pop_temporary(tmp_reg);
	else
		release_reg();
}