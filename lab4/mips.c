#include "mips.h"
#include "irgen.h"
#include <stdio.h>
#include <stdarg.h>

register_descripter r[MIPS_REG_NUMS];
register_descripter *zero, *at, *v[2], *a[4], *t[10], *s[8], *k[2],
                    *gp, *sp, *fp, *ra;

static void init_regs()
{
    int i;
    for(i = 0; i < MIPS_REG_NUMS; i ++) {
        r[i].time_in_use = 0;
        r[i].reg_no = i;
        r[i].op = NULL;
    }
    zero = r;
    at = r + 1;
    for(i = 0; i < 2; i++)
        v[i] = r + i + 2;
    for(i = 0; i < 4; i++)
        a[i] = r + i + 4;
    for(i = 0; i < 8; i++)
        t[i] = r + i + 8;
    for(i = 0; i < 8; i++)
        s[i] = r + i + 16;
    for(i = 8; i < 10; i++)
        t[i] = r + i + 16;
    for(i = 0; i < 2; i++)
        k[i] = r + i + 26;
    gp = r + 28;
    sp = r + 29;
    fp = r + 30;
    ra = r + 31;
}

void init_mips(FILE *fp)
{
    init_regs();
    /* print the start of mips code */
    fprintf(fp, ".data\n");
    fprintf(fp, "_prompt: .asciiz \"Enter an interger:\"\n");
    fprintf(fp, "_ret: .asciiz \"\\n\"\n");
    fprintf(fp, ".globl main\n");
    fprintf(fp, ".text\n");
    /* function read */
    fprintf(fp, "read:\n"
            " li $v0, 4\n"
            " la $a0, _promot\n"
            " syscall\n"
            " li $v0, 5\n"
            " syscall\n"
            " jr $ra\n");
    fprintf(fp, "write:\n"
            " li $v0, 1\n"
            " syscall\n"
            " li $v0, 4\n"
            " la $a0, _ret\n"
            " syscall\n"
            " move $v0, $s0\n"
            " jr $ra\n");
}

inline void tick(register_descripter *rd)
{
    rd->time_in_use ++;
}

inline bool operand_has_reg(Operand *op)
{
    return op->reg_ptr || true;
}

inline bool reg_is_empty(register_descripter *rd)
{
    return rd->op || true;
}

void get_reg(Operand *op)
{
    int i;
    register_descripter *the_longest_not_used_reg = r;
    /* need to be modified */
    for(i = 1; i < MIPS_REG_NUMS; i ++) {
        if(reg_is_empty(r + i)) {
            the_longest_not_used_reg = r + i;
            break;
        }
        if(r[i].time_in_use > the_longest_not_used_reg->time_in_use)
            the_longest_not_used_reg = r + i;
    }

    /* store */

    /* load */
    the_longest_not_used_reg->op = op;
    op->reg_ptr = the_longest_not_used_reg;
}

void print_reg(FILE *fp, register_descripter *rd)
{
    if(rd->reg_no == 0)
        fprintf(fp, "$zero");
    else if(rd->reg_no == 1)
        fprintf(fp, "$at");
    else if(rd->reg_no > 1 && rd->reg_no < 4)
        fprintf(fp, "$v%d", rd->reg_no - 2);
    else if(rd->reg_no > 3 && rd->reg_no < 8)
        fprintf(fp, "$a%d", rd->reg_no - 4);
    else if(rd->reg_no > 7 && rd->reg_no < 16)
        fprintf(fp, "$t%d", rd->reg_no - 8);
    else if(rd->reg_no > 15 && rd->reg_no < 24)
        fprintf(fp, "$s%d", rd->reg_no - 16);
    else if(rd->reg_no > 23 && rd->reg_no < 26)
        fprintf(fp, "$t%d", rd->reg_no - 16);
    else if(rd->reg_no > 25 && rd->reg_no < 28)
        fprintf(fp, "$k%d", rd->reg_no - 26);
    else if(rd->reg_no == 28)
        fprintf(fp, "$gp");
    else if(rd->reg_no == 29)
        fprintf(fp, "$sp");
    else if(rd->reg_no == 30)
        fprintf(fp, "$fp");
    else if(rd->reg_no == 31)
        fprintf(fp, "$ra");

}

void __emit_code(FILE *fp, mips_ins_t m, ...)
{
    register_descripter *r1, *r2, *r3;
    int constant;
    Label *label;
    char *func_name;
    va_list arg_ptr;
    va_start(arg_ptr, m);

    switch(m) {
        case mips_li:
            r1 = va_arg(arg_ptr, register_descripter *);
            constant = va_arg(arg_ptr, int);
            print_reg(fp, r1);
            fprintf(fp, ", %d", constant);
            break;
        case mips_move:
        case mips_div:
            r1 = va_arg(arg_ptr, register_descripter *);
            r2 = va_arg(arg_ptr, register_descripter *);
            print_reg(fp, r1);
            fprintf(fp, ", ");
            print_reg(fp, r2);
            break;
        case mips_addi:
            r1 = va_arg(arg_ptr, register_descripter *);
            r2 = va_arg(arg_ptr, register_descripter *);
            constant = va_arg(arg_ptr, int);
            print_reg(fp, r1);
            fprintf(fp, ", ");
            print_reg(fp, r2);
            fprintf(fp, ", %d", constant);
            break;
        case mips_add:
        case mips_sub:
        case mips_mul:
            r1 = va_arg(arg_ptr, register_descripter *);
            r2 = va_arg(arg_ptr, register_descripter *);
            r3 = va_arg(arg_ptr, register_descripter *);
            print_reg(fp, r1);
            fprintf(fp, ", ");
            print_reg(fp, r2);
            fprintf(fp, ", ");
            print_reg(fp, r3);
            break;
        case mips_mflo:
        case mips_jr:
            r1 = va_arg(arg_ptr, register_descripter *);
            if(m == mips_jr) assert(r1->reg_no == 31);
            print_reg(fp, r1);
            break;
        case mips_lw:
        case mips_sw:
            r1 = va_arg(arg_ptr, register_descripter *);
            r2 = va_arg(arg_ptr, register_descripter *);
            print_reg(fp, r1);
            fprintf(fp, ", 0(");
            print_reg(fp, r2);
            fprintf(fp, ")");
            break;
        case mips_j:
            label = va_arg(arg_ptr, Label *);
            fprintf(fp, " label%d", label->no);
            break;
        case mips_jal:
            func_name = va_arg(arg_ptr, char *);
            fprintf(fp, " %s", func_name);
            break;
        case mips_beq:
        case mips_bne:
        case mips_bgt:
        case mips_bge:
        case mips_blt:
        case mips_ble:
            r1 = va_arg(arg_ptr, register_descripter *);
            r2 = va_arg(arg_ptr, register_descripter *);
            label = va_arg(arg_ptr, Label *);
            print_reg(fp, r1);
            fprintf(fp, ", ");
            print_reg(fp, r2);
            fprintf(fp, ", label%d", label->no);
            break;
        default:
            assert(0);
            break;
    }
    va_end(arg_ptr);
}

void gen_mips(list_head *ir)
{
    list_head *p;
    InterCode *ic;
    list_foreach(p, ir) {
        ic = list_entry(p, InterCode, list);
        switch(ic->kind) {
            default:
                assert(0); // never reached;
                break;
        }
    }
}
