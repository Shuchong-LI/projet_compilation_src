#include "miniccutils.h"
#include <stdio.h>

#include "defs.h"
#include "passe_2.h"

/* Notes : quand on load ou on store une valeur et que la variable est sur le stack
 * on peut le faire en une instruction de moins en utilisant $sp
 */


void print_handler(node_t root);
void block_allocation(node_t root);
void expression_handler(node_t root);
void for_handler(node_t root);
void while_handler(node_t root);
void do_while_handler(node_t root);
void if_handler(node_t root);

int label_num = 1;

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
		create_inst_label_str("main");
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

	case NODE_IDENT: // Ya des trucs bizarres ici, mais ça a l'air de marcher
		// Première déclaration
		if (root->decl_node == NULL) {
			// Variable globale
			if (root->global_decl)
				create_inst_word(root->ident, root->value);
			root->decl_node = root;
		}
		else
			expression_handler(root);
		break;

	case NODE_INTVAL : case NODE_BOOLVAL:
		expression_handler(root);
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
		create_inst_comment("print");
		print_handler(root->opr[0]);
		break;

	case NODE_FOR:
		create_inst_comment("for");
		for_handler(root);
		break;

	case NODE_WHILE:
		create_inst_comment("while");
		while_handler(root);
		break;

	case NODE_DOWHILE:
		create_inst_comment("do while");
		do_while_handler(root);
		break;

	case NODE_IF:
		create_inst_comment("if");
		if_handler(root);
		break;

	case NODE_AFFECT:
		create_inst_comment("affect");

		// Right
		expression_handler(root->opr[1]);
		int32_t rreg = get_current_reg();
		int rreg_available = reg_available();
		if (rreg_available)
			allocate_reg();
		else
			push_temporary(rreg);

		// Left : get address of variable
		int32_t address_reg = get_current_reg();
		// Get address of lvalue
		if (root->opr[0]->global_decl)
			create_inst_lui(address_reg, DATA_SECTION_BASE_ADDRESS);	// Fetch global address
		else
			create_inst_or(address_reg, $zero, $sp);			// Fetch stack address

		// Retrieve rvalue
		if (!rreg_available) {
			rreg = get_restore_reg();
			pop_temporary(rreg);
		}
		// Store rvalue into lvalue
		create_inst_sw(rreg, root->opr[0]->offset, address_reg);

		if (rreg_available)
			release_reg();

		break;

	case NODE_PLUS: case NODE_MINUS: case NODE_MUL: case NODE_DIV: case NODE_MOD:
	case NODE_LT: case NODE_GT:
		expression_handler(root);
		break;

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
		create_inst_ori($v0, $zero, PRINT_STRING_SYSCALL);		// $v0 <- 4
		create_inst_lui($a0, DATA_SECTION_BASE_ADDRESS);		// fetch global address
		create_inst_addiu($a0, $a0, root->offset);			// add the offset
		create_inst_syscall();
	} else {
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
		if (root->opr[1] != NULL) {
			expression_handler(root->opr[1]);
			int32_t tmp_reg = get_current_reg();
			create_inst_sw(tmp_reg, root->opr[0]->offset, $sp);
		}
		// TODO : operation
		break;
	}
}

void expression_handler(node_t root)
{
	if (root == NULL)
		return;

	int32_t tmp_reg;
	switch (root->nature) {
	case NODE_INTVAL: case NODE_BOOLVAL:
		tmp_reg = get_current_reg();
		create_inst_ori(tmp_reg, $zero, root->value);
		return;

	case NODE_IDENT:
		int32_t add_reg = get_current_reg();

		if (root->global_decl)
			create_inst_lui(add_reg, DATA_SECTION_BASE_ADDRESS);
		else
			create_inst_or(add_reg, $zero, $sp);

		create_inst_lw(add_reg, root->offset, add_reg);
		return;

	case NODE_UMINUS:
		expression_handler(root->opr[0]);
		tmp_reg = get_current_reg();
		create_inst_subu(tmp_reg, $zero, tmp_reg);
		return;

	default:
	}
	// Else it's an expression

	// Left
	expression_handler(root->opr[0]);
	int32_t lreg = get_current_reg();
	int lreg_available = reg_available();
	if (lreg_available)
		allocate_reg();
	else
		push_temporary(lreg);

	// Right
	expression_handler(root->opr[1]);
	
	// Operation
	if (!lreg_available) {
		lreg = get_restore_reg();
		pop_temporary(lreg);
	}
	int32_t rreg = get_current_reg();

	switch (root->nature) {
	case NODE_PLUS:
		if (lreg_available) {				// TODO : check for overflow
			create_inst_addu(lreg, lreg, rreg);
			release_reg();
		} else
			create_inst_addu(rreg, lreg, rreg);
		break;

	case NODE_MINUS:
		if (lreg_available) {				// TODO : check for overflow
			create_inst_subu(lreg, lreg, rreg);
			release_reg();
		} else
			create_inst_subu(rreg, lreg, rreg);
		break;

	case NODE_LT:
		if (lreg_available) {
			create_inst_slt(lreg, lreg, rreg);
			release_reg();
		} else
			create_inst_slt(rreg, lreg, rreg);
		break;
	}
}

void for_handler(node_t root)
{
	int for_start = label_num++;
	int for_end = label_num++;
	gen_code_passe_2(root->opr[0]);
	create_inst_label(for_start);				// For loop begin
	gen_code_passe_2(root->opr[1]);				// Expression
	create_inst_beq(get_current_reg(), $zero, for_end);	// Check condition
	gen_code_passe_2(root->opr[2]);				// Inside loop
	gen_code_passe_2(root->opr[3]);				// Last instruction
	create_inst_j(for_start);
	create_inst_label(for_end);				// For loop end
}

void while_handler(node_t root)
{
	int while_start = label_num++;
	int while_end = label_num++;
	create_inst_label(while_start);
	gen_code_passe_2(root->opr[0]);				// Check condition
	create_inst_beq(get_current_reg(), $zero, while_end);
	gen_code_passe_2(root->opr[1]);
	create_inst_j(while_start);
	create_inst_label(while_end);
}

void do_while_handler(node_t root)
{
	int loop_start = label_num++;
	create_inst_label(loop_start);
	gen_code_passe_2(root->opr[0]);
	gen_code_passe_2(root->opr[1]);
	create_inst_bne(get_current_reg(), $zero, loop_start);
}

void if_handler(node_t root)
{
	int false_label = label_num++;
	int end_if_label = label_num++;
	gen_code_passe_2(root->opr[0]);					// Condition
	create_inst_beq(get_current_reg(), $zero, false_label);
	gen_code_passe_2(root->opr[1]);
	create_inst_j(end_if_label);
	create_inst_label(false_label);
	if (root->nops > 2)
		gen_code_passe_2(root->opr[2]);
	create_inst_label(end_if_label);
}