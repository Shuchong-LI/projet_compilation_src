#include "miniccutils.h"
#include <stdio.h>

#include "defs.h"
#include "passe_2.h"

void gen_code_passe_2(node_t root) {
	if (root == NULL)
		return;
	
	switch (root->nature) {
	case NODE_PROGRAM:
		create_inst_data_sec();
		create_inst_label_str("data_section_start");
		gen_code_passe_2(root->opr[0]);
		// Allocation des chaines des caracteres
		for (int i = 0; i < get_global_strings_number(); i++)
			create_inst_asciiz(NULL, get_global_string(i));

		create_inst_text_sec();
		gen_code_passe_2(root->opr[1]);

		create_inst_comment("exit");
		create_inst_ori($v0, $zero, 10); // $v0 <- 10
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
		gen_code_passe_2(root->opr[2]);
		break;
	
	case NODE_BLOCK: //TODO
		gen_code_passe_2(root->opr[1]);
		break;

	case NODE_PRINT: // TODO
		create_inst_ori($v0, $zero, 4); // $v0 <- 4
		gen_code_passe_2(root->opr[0]);
		create_inst_syscall();
		break;

	case NODE_STRINGVAL:
		create_inst_lui($a0, DATA_SECTION_BASE_ADDRESS);
		create_inst_addiu($a0, $a0, root->offset);
		break;
	default:
		break;
	}
}

