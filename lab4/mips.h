#ifndef __MIPS_H__
#define __MIPS_H__

#include "tree.h"
#include "bool.h"
#include "irgen.h"

#define __typedef(x) typedef struct x x;

struct register_descripter {
    int reg_no;
    int time_in_use;
    Operand *op;
};
__typedef(register_descripter);

typedef enum mips_instruction_types {
    mips_li, mips_move, mips_addi, mips_add, mips_sub, mips_mul, mips_div, mips_mflo,
    mips_lw, mips_sw, mips_j, mips_jal, mips_jr,
    mips_beq, mips_bne, mips_bgt, mips_bge, mips_blt, mips_ble
} mips_ins_t;

#define MIPS_REG_NUMS   32
void init_mips(FILE *);
void tick(register_descripter *);
bool operand_has_reg(Operand *);
bool reg_is_empty(register_descripter *);
void get_reg(Operand *op);
void gen_mips(list_head *ir);
void __emit_code(FILE *fp, mips_ins_t, ...);
#define emit_code(m, ...) { \
    printf(#m); printf(" "); \
    __emit_code(stdout, mips_##m, __VA_ARGS__) \
    printf("\n"); \
}

#endif
