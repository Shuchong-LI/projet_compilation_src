#include "miniccutils.h"
#include <stdio.h>

#include "defs.h"
#include "passe_2.h"

void print_handler(node_t root);
void block_allocation(node_t root);
void right_affect_handler(node_t root, int32_t address_reg);
void expression_handler(node_t root, int32_t result_reg);

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
		int32_t address_reg;
		int is_reg_available = reg_available();
		// Allocate register for address
		if (is_reg_available) {
			allocate_reg();
			address_reg = get_current_reg();
		} else {
			address_reg = get_restore_reg();
			push_temporary(address_reg);
		}

		// Get address of lvalue
		if (root->opr[0]->global_decl)
			create_inst_lui(address_reg, DATA_SECTION_BASE_ADDRESS);	// Fetch global address
		else
			create_inst_or(address_reg, $zero, $sp);			// Fetch stack address
		create_inst_addiu(address_reg, address_reg, root->offset);		// Add the offset
		
		right_affect_handler(root->opr[1], address_reg);
		//create_inst_sw(address_reg, root->opr[0]->offset, result_reg);	// Store right value of affect
										// in memory

		if (is_reg_available)
			release_reg();
		else
			pop_temporary(address_reg);
		//reg_desallocation(address_reg, is_reg_available);
		break;

	case NODE_PLUS:


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

int32_t affect_handler(node_t root)
{
	switch (root->nature) {
	default:
		return 1;
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
		// Allocate registre
		if (is_reg_available) {
			allocate_reg();
			tmp_reg = get_current_reg();
		} else {
			tmp_reg = get_restore_reg();
			push_temporary(tmp_reg);
		}

		create_inst_ori(tmp_reg, $zero, root->opr[0]->value);
		create_inst_sw(tmp_reg, root->opr[0]->offset, $sp);

		// Release registre
		if (is_reg_available)
			release_reg();
		else
			pop_temporary(tmp_reg);
		// TODO : operation
		break;
	}
}

void right_affect_handler(node_t root, int32_t address_reg)
{
	int32_t tmp_reg;
	int is_reg_available = reg_available();
	// Allocate registre
	if (is_reg_available) {
		allocate_reg();
		tmp_reg = get_current_reg();
	} else {
		tmp_reg = get_restore_reg();
		push_temporary(tmp_reg);
	}

	switch (root->nature) {
	case NODE_INTVAL : case NODE_BOOLVAL:
		create_inst_ori(tmp_reg, $zero, root->value);		// Load value into tmp reg
		create_inst_sw(tmp_reg, 0, address_reg);		// Store value into variable
		break;

	case NODE_IDENT:
		if (root->global_decl)
			create_inst_lui(tmp_reg, DATA_SECTION_BASE_ADDRESS);	// Fetch global address
		else
			create_inst_or(tmp_reg, $zero, $sp);			// Fetch stack address
		create_inst_addiu(tmp_reg, tmp_reg, root->offset);		// Add offset
		create_inst_lw(tmp_reg, 0, tmp_reg);				// Load value into registre
		create_inst_sw(tmp_reg, 0, address_reg);			// Store value into variable
		break;
	
	default:
		expression_handler(root, tmp_reg);
		create_inst_sw(tmp_reg, 0, address_reg);
		break;
	}
	// Release registre
	if (is_reg_available)
		release_reg();
	else
		pop_temporary(tmp_reg);
	return;
}

void expression_handler(node_t root, int32_t result_reg)
{
	if (root->nature == NODE_BOOLVAL || root->nature == NODE_INTVAL) {
		create_inst_ori(result_reg, $zero, root->value);
		return;
	}

	int32_t tmp_reg;
	int is_reg_available = reg_available();
	// Allocate registre
	if (is_reg_available) {
		allocate_reg();
		tmp_reg = get_current_reg();
	} else {
		create_inst_comment("push");
		tmp_reg = get_restore_reg();
		push_temporary(tmp_reg);
	}

	switch (root->nature) {
	case NODE_PLUS:
		create_inst_comment("plus");
		expression_handler(root->opr[0], result_reg);
		expression_handler(root->opr[1], tmp_reg);
		create_inst_addu(result_reg, result_reg, tmp_reg); // TODO : Check overflow
		break;
	case NODE_IDENT:
		if (root->global_decl)
			create_inst_lui(tmp_reg, DATA_SECTION_BASE_ADDRESS);	// Fetch global address
		else
			create_inst_or(tmp_reg, $zero, $sp);			// Fetch stack address
		create_inst_lw(result_reg, root->offset, tmp_reg);			// Add offset and load value
		break;
	}

	// Release registre
	if (is_reg_available)
		release_reg();
	else {
		create_inst_comment("pop");
		pop_temporary(tmp_reg);
	}
	return;
}