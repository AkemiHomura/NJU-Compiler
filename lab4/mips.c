#include "mips.h"
#include "irgen.h"
#include "ta.h"
#include <stdio.h>
#include <stdarg.h>

register_descripter r[MIPS_REG_NUMS];
register_descripter *_zero, *at, *v[2], *a[4], *t[10], *s[8], *k[2],
                    *gp, *sp, *_fp, *ra;

FILE *mips_fp;
#define emit_code(m, ...) { \
    __emit_code(mips_fp, mips_##m, __VA_ARGS__); \
}

static int func_stack;

static void init_regs()
{
    int i;
    for(i = 0; i < MIPS_REG_NUMS; i ++) {
        r[i].time_in_use = 0;
        r[i].reg_no = i;
        r[i].op = NULL;
    }
    _zero = r;
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
    _fp = r + 30;
    ra = r + 31;
}

void init_mips(FILE *fp)
{
    mips_fp = fp;
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
            " la $a0, _prompt\n"
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

void print_reg(FILE *fp, register_descripter *rd)
{
    //fprintf(fp, "$%d", rd->reg_no);
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
    r1 = r2 = r3 = NULL;
    int constant;
    Label *label;
    char *func_name;
    va_list arg_ptr;
    va_start(arg_ptr, m);

    fprintf(fp, " ");
    switch(m) {
        case mips_li:
            fprintf(fp, "li ");
            r1 = va_arg(arg_ptr, register_descripter *);
            constant = va_arg(arg_ptr, int);
            print_reg(fp, r1);
            fprintf(fp, ", %d", constant);
            break;
        case mips_move:
        case mips_div:
            if(m == mips_move) fprintf(fp, "move ");
            else fprintf(fp, "div ");
            r1 = va_arg(arg_ptr, register_descripter *);
            r2 = va_arg(arg_ptr, register_descripter *);
            print_reg(fp, r1);
            fprintf(fp, ", ");
            print_reg(fp, r2);
            break;
        case mips_addi:
            fprintf(fp, "addi ");
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
            if(m == mips_add) fprintf(fp, "add ");
            else if(m == mips_sub) fprintf(fp, "sub ");
            else fprintf(fp, "mul ");
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
            if(m == mips_mflo) fprintf(fp, "mflo ");
            else fprintf(fp, "jr ");
            r1 = va_arg(arg_ptr, register_descripter *);
            if(m == mips_jr) assert(r1->reg_no == 31);
            print_reg(fp, r1);
            break;
        case mips_lw:
        case mips_sw:
            if(m == mips_lw) fprintf(fp, "lw ");
            else fprintf(fp, "sw ");
            r1 = va_arg(arg_ptr, register_descripter *);
            constant = va_arg(arg_ptr, int);
            r2 = va_arg(arg_ptr, register_descripter *);
            print_reg(fp, r1);
            fprintf(fp, ", %d(", constant);
            print_reg(fp, r2);
            fprintf(fp, ")");
            break;
        case mips_j:
            fprintf(fp, "j ");
            label = va_arg(arg_ptr, Label *);
            fprintf(fp, " label%d", label->no);
            break;
        case mips_jal:
            fprintf(fp, "jal ");
            func_name = va_arg(arg_ptr, char *);
            fprintf(fp, " %s", func_name);
            break;
        case mips_beq:
        case mips_bne:
        case mips_bgt:
        case mips_bge:
        case mips_blt:
        case mips_ble:
            if(m == mips_beq) fprintf(fp, "beq ");
            else if(m == mips_bne) fprintf(fp, "bne ");
            else if(m == mips_bgt) fprintf(fp, "bgt ");
            else if(m == mips_bge) fprintf(fp, "bge ");
            else if(m == mips_blt) fprintf(fp, "blt ");
            else fprintf(fp, "ble ");
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
    fprintf(fp, "\n");
}

void print_ic(FILE*, InterCode *);

void load_t(register_descripter *t, Operand *op)
{
    if(op->kind == CONSTANT) {
        emit_code(li, t, op->u.cons);
    } else if(op->kind == VARIABLE ||
            op->kind == TEMPVAR) {
        emit_code(lw, t, func_stack - op->offset2fp, sp);
    } else if(op->kind == VARIADDR) {
        emit_code(addi, t, sp, func_stack - op->offset2fp);
    }
}

void store_t(register_descripter *t, Operand *op)
{
    assert(op->kind != VARIADDR && op->kind != LABEL &&
            op->kind != FUNCTION);
    emit_code(sw, t, func_stack - op->offset2fp, sp);
}

void gen_mips(list_head *ir, FILE *fp)
{
    list_head *p;
    InterCode *ic;
    list_foreach(p, ir) {
        ic = list_entry(p, InterCode, list);
        print_ic(stdout, ic);
        int func_stack;
        Operand *func;
        switch(ic->kind) {
            case IR_FUNCTION:
                ic->u.one.op->offset2fp = func_stack = 0;
                func = ic->u.one.op;
                break;
            case IR_RETURN:
            case IR_LABEL:
            case IR_GOTO:
            case IR_READ:
            case IR_WRITE:
            case IR_ARG:
                if(ic->u.one.op->kind == VARIABLE ||
                        ic->u.one.op->kind == TEMPVAR) {
                    if(ic->u.one.op->offset2fp < 0) {
                        ic->u.one.op->offset2fp = func_stack + 4;
                        func_stack += 4;
                        func->offset2fp += 4;
                    }
                }
                break;
            case IR_PARAM:
                {
                    if(ic->u.one.op->kind == VARIABLE ||
                            ic->u.one.op->kind == TEMPVAR) {
                        if(ic->u.one.op->offset2fp < 0) {
                            ic->u.one.op->offset2fp = func_stack + 4;
                            func_stack += 4;
                            func->offset2fp += 4;
                        }
                    }
                }
                break;
            case IR_ASSIGN:
            case IR_CALL:
            case IR_LEFTSTAR:
            case IR_RIGHTSTAR:
                if(ic->u.assign.left->kind == VARIABLE ||
                        ic->u.assign.left->kind == TEMPVAR) {
                    if(ic->u.assign.left->offset2fp < 0) {
                        ic->u.assign.left->offset2fp = func_stack + 4;
                        func_stack += 4;
                        func->offset2fp += 4;
                    }
                }
                if(ic->u.assign.right->kind == VARIABLE ||
                        ic->u.assign.right->kind == TEMPVAR) {
                    if(ic->u.assign.right->offset2fp < 0) {
                        ic->u.assign.right->offset2fp = func_stack + 4;
                        func_stack += 4;
                        func->offset2fp += 4;
                    }
                }
                break;
            case IR_ADD:
            case IR_SUB:
            case IR_MUL:
            case IR_DIV:
                if(ic->u.binop.result->kind == VARIABLE ||
                        ic->u.binop.result->kind == TEMPVAR) {
                    if(ic->u.binop.result->offset2fp < 0) {
                        ic->u.binop.result->offset2fp = func_stack + 4;
                        func_stack += 4;
                        func->offset2fp += 4;
                    }
                }
                if(ic->u.binop.op1->kind == VARIABLE ||
                        ic->u.binop.op1->kind == TEMPVAR) {
                    if(ic->u.binop.op1->offset2fp < 0) {
                        ic->u.binop.op1->offset2fp = func_stack + 4;
                        func_stack += 4;
                        func->offset2fp += 4;
                    }
                }
                if(ic->u.binop.op2->kind == VARIABLE ||
                        ic->u.binop.op2->kind == TEMPVAR) {
                    if(ic->u.binop.op2->offset2fp < 0) {
                        ic->u.binop.op2->offset2fp = func_stack + 4;
                        func_stack += 4;
                        func->offset2fp += 4;
                    }
                }
                break;
            case IR_IFGOTO:
                break;
            case IR_DEC:
                if(ic->u.dec.op->kind == VARIABLE ||
                        ic->u.dec.op->kind == TEMPVAR) {
                    ic->u.dec.op->offset2fp = func_stack + ic->u.dec.size;
                    func_stack += ic->u.dec.size;
                    func->offset2fp += ic->u.dec.size;
                }
                break;
            case IR_RIGHTAT:
                ic->u.assign.left->offset2fp = ic->u.assign.right->offset2fp;
                break;
        }
    }
    list_foreach(p, ir) {
        ic = list_entry(p, InterCode, list);
        switch(ic->kind) {
            case IR_LABEL:
                fprintf(fp, "label%d:\n", ic->u.one.op->u.label->no);
                break;
            case IR_ASSIGN:
                // x := #k
                load_t(t[0], ic->u.assign.left);
                if(ic->u.assign.right->kind == CONSTANT) {
                    emit_code(li, t[0],
                            ic->u.assign.right->u.cons);
                }
                // x := y
                else {
                    load_t(t[1], ic->u.assign.right);
                    emit_code(move, t[0], t[1]);
                }
                emit_code(sw, t[0], func_stack - ic->u.assign.left->offset2fp, sp);
                break;
            case IR_ADD:
            case IR_SUB:
            case IR_MUL:
            case IR_DIV:
                load_t(t[0], ic->u.binop.result);
                load_t(t[1], ic->u.binop.op1);
                load_t(t[2], ic->u.binop.op2);
                if(ic->kind == IR_ADD) {
                    emit_code(add, t[0], t[1], t[2]);
                }
                else if(ic->kind == IR_SUB) {
                    emit_code(sub, t[0], t[1], t[2]);
                }
                else if(ic->kind == IR_MUL) {
                    emit_code(mul, t[0], t[1], t[2]);
                }
                else {
                    emit_code(div, t[1], t[2]);
                    emit_code(mflo, t[0]);
                }
                store_t(t[0], ic->u.binop.result);
                break;
            case IR_RETURN:
                load_t(t[0], ic->u.one.op);
                emit_code(move, v[0], t[0]);
                emit_code(jr, ra);
                break;
            case IR_GOTO:
                emit_code(j, ic->u.one.op->u.label);
                break;
            case IR_IFGOTO:
                if(ic->u.triop.t1->reg_ptr) {
                    emit_code(move, t[0], ic->u.triop.t1->reg_ptr);
                } else if(ic->u.triop.t1->kind == CONSTANT) {
                    emit_code(li, t[0], ic->u.triop.t1->u.cons);
                } else
                    emit_code(lw, t[0], func_stack - ic->u.triop.t1->offset2fp, sp);
                if(ic->u.triop.t2->reg_ptr) {
                    emit_code(move, t[1], ic->u.triop.t2->reg_ptr);
                } else if(ic->u.triop.t2->kind == CONSTANT) {
                    emit_code(li, t[1], ic->u.triop.t2->u.cons);
                } else
                    emit_code(lw, t[1], func_stack - ic->u.triop.t2->offset2fp, sp);
                if(ic->u.triop.relop == EQ) {
                    emit_code(beq, t[0], t[1],
                            ic->u.triop.label->u.label);
                } else if(ic->u.triop.relop == NE) {
                    emit_code(bne, t[0], t[1],
                            ic->u.triop.label->u.label);
                } else if(ic->u.triop.relop == GT) {
                    emit_code(bgt, t[0], t[1],
                            ic->u.triop.label->u.label);
                } else if(ic->u.triop.relop == LT) {
                    emit_code(blt, t[0], t[1],
                            ic->u.triop.label->u.label);
                } else if(ic->u.triop.relop == GE) {
                    emit_code(bge, t[0], t[1],
                            ic->u.triop.label->u.label);
                } else {
                    emit_code(ble, t[0], t[1],
                            ic->u.triop.label->u.label);
                }
                break;
            case IR_READ:
                mips_pushall();
                emit_code(jal, "read");
                mips_popall();
                load_t(t[0], ic->u.one.op);
                emit_code(move, t[0], v[0]);
                store_t(t[0], ic->u.one.op);
                break;
            case IR_WRITE:
                load_t(t[0], ic->u.one.op);
                emit_code(move, a[0], t[0]);
                mips_pushall();
                emit_code(jal, "write");
                mips_popall();
                break;
            case IR_FUNCTION:
                fprintf(fp, "%s:\n", ic->u.one.op->u.value);
                func_stack = ic->u.one.op->offset2fp;
                emit_code(addi, sp, sp, -func_stack);
                /* need to be filled */
                break;
            case IR_LEFTSTAR:
                load_t(t[0], ic->u.assign.right);
                load_t(t[1], ic->u.assign.left);
                emit_code(sw, t[0], 0, t[1]);
                break;
            case IR_RIGHTSTAR:
                load_t(t[0], ic->u.assign.left);
                load_t(t[1], ic->u.assign.right);
                emit_code(lw, t[0], 0, t[1]);
                store_t(t[0], ic->u.assign.left);
                break;
            case IR_ARG:
                /* self loop until function */
                mips_pushall();
                {
                    InterCode *ir_call = ic;
                    list_head *q = p;
                    while(ir_call->kind != IR_CALL) {
                        q = q->next;
                        ir_call = list_entry(q, InterCode, list);
                    }
                    symbol *s = find_by_name_global(ir_call->u.assign.right->u.value);
                    int offset = 4;
                    Operand *arg = list_entry(q->prev, InterCode, list)->u.one.op;
                    while(arg) {
                        load_t(t[0], arg);
                        emit_code(sw, t[0], -offset, _fp);
                        offset += 4;
                        arg = arg->next;
                    }
                    /* call */
                    emit_code(jal, ir_call->u.assign.right->u.value);
                    mips_popall();
                    load_t(t[0], ir_call->u.assign.left);
                    emit_code(move, t[0], v[0]);
                    store_t(t[0], ir_call->u.assign.left);
                    p = q;
                }
                break;
            case IR_CALL:
                mips_pushall();
                emit_code(jal, ic->u.assign.right->u.value);
                mips_popall();
                load_t(t[0], ic->u.assign.left);
                emit_code(move, t[0], v[0]);
                store_t(t[0], ic->u.assign.left);
                break;
            /* not to deal with */
            case IR_RIGHTAT:
            case IR_PARAM:
            case IR_DEC:
                break;
            default:
                assert(0); // never reached;
                break;
        }
    }
}

void mips_pushall()
{
    emit_code(addi, sp, sp, -8);
    func_stack += 8;
    emit_code(sw, ra, 0, sp);
    emit_code(sw, _fp, 4, sp);
    emit_code(move, _fp, sp);
}

void mips_popall()
{
    emit_code(move, sp, _fp);
    emit_code(lw, _fp, 4, sp);
    emit_code(lw, ra, 0, sp);
    emit_code(addi, sp, sp, 8);
    func_stack -= 8;
}
