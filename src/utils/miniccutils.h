#ifndef _MINICCUTILS_H_
#define _MINICCUTILS_H_

#include <stdint.h>
#include <stdbool.h>

#include "../defs.h"



/* For checking the tree constructed */

bool check_program_tree(node_t n);


/* Context related functions */

typedef struct _context_s context_s;
typedef context_s * context_t;

context_t create_context();
void free_context(context_t context);
bool context_add_element(context_t context, char * idf, void * data);
void * get_data(context_t context, char * idf);


/* Environment related functions */

void push_global_context();
void push_context();
void pop_context();
void * get_decl_node(char * ident);
int32_t env_add_element(char * ident, void * node);
void reset_env_current_offset();
int32_t get_env_current_offset();
int32_t add_string(char * str);
int32_t get_global_strings_number();
char * get_global_string(int32_t index);
void free_global_strings();


/* Register allocation related functions */

void push_temporary(int32_t reg);
void pop_temporary(int32_t reg);
bool reg_available();
int32_t get_current_reg();
int32_t get_restore_reg();
void allocate_reg();
void release_reg();
int32_t get_new_label();
void set_temporary_start_offset(int32_t offset);
void set_max_registers(int32_t n);
void reset_temporary_max_offset();
int32_t get_temporary_max_offset();
int32_t get_temporary_curr_offset(); // for debug


/* Program creation related functions */

void create_inst_data_sec();
void create_inst_text_sec();
void create_inst_word(char * label, int32_t init_value);
void create_inst_asciiz(char * label_str, char * str);
void create_inst_label(int32_t label);
void create_inst_label_str(char * label);
void create_inst_comment(char * comment);
void create_inst_lui(int32_t r_dest, int32_t imm);
void create_inst_addu(int32_t r_dest, int32_t r_src_1, int32_t r_src_2);
void create_inst_subu(int32_t r_dest, int32_t r_src_1, int32_t r_src_2);
void create_inst_slt(int32_t r_dest, int32_t r_src_1, int32_t r_src_2);
void create_inst_sltu(int32_t r_dest, int32_t r_src_1, int32_t r_src_2);
void create_inst_and(int32_t r_dest, int32_t r_src_1, int32_t r_src_2);
void create_inst_or(int32_t r_dest, int32_t r_src_1, int32_t r_src_2);
void create_inst_xor(int32_t r_dest, int32_t r_src_1, int32_t r_src_2);
void create_inst_nor(int32_t r_dest, int32_t r_src_1, int32_t r_src_2);
void create_inst_mult(int32_t r_src_1, int32_t r_src_2);
void create_inst_div(int32_t r_src_1, int32_t r_src_2);
void create_inst_sllv(int32_t r_dest, int32_t r_src_1, int32_t r_src_2);
void create_inst_srav(int32_t r_dest, int32_t r_src_1, int32_t r_src_2);
void create_inst_srlv(int32_t r_dest, int32_t r_src_1, int32_t r_src_2);
void create_inst_addiu(int32_t r_dest, int32_t r_src_1, int32_t imm);
void create_inst_andi(int32_t r_dest, int32_t r_src_1, int32_t imm);
void create_inst_ori(int32_t r_dest, int32_t r_src_1, int32_t imm);
void create_inst_xori(int32_t r_dest, int32_t r_src_1, int32_t imm);
void create_inst_slti(int32_t r_dest, int32_t r_src_1, int32_t imm);
void create_inst_sltiu(int32_t r_dest, int32_t r_src_1, int32_t imm);
void create_inst_lw(int32_t r_dest, int32_t imm, int32_t r_src_1);
void create_inst_sw(int32_t r_src_1, int32_t imm, int32_t r_src_2);
void create_inst_beq(int32_t r_src_1, int32_t r_src_2, int32_t label);
void create_inst_bne(int32_t r_src_1, int32_t r_src_2, int32_t label);
void create_inst_mflo(int32_t r_dest);
void create_inst_mfhi(int32_t r_dest);
void create_inst_j(int32_t label);
void create_inst_teq(int32_t r_src_1, int32_t r_src_2);
void create_inst_syscall();
void create_inst_stack_allocation();
void create_inst_stack_deallocation(int32_t val);

void create_program();
void free_program();
void dump_mips_program(char * filename);




#endif

