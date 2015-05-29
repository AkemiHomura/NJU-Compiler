#include <stdio.h>
#include <stdlib.h>
#include "irgen.h"
#include "ta.h"
#include "tree.h"

extern Operand fall;

list_head code;

Operand zero = {CONSTANT, .u.cons = 0, NULL};
Operand one = {CONSTANT, .u.cons = 1, NULL};

void init_irgen() {
    list_init(&code);
}

void export_code(InterCode *c) {
    list_init(&c->list);
    list_add_before(&code, &c->list);
}

InterCode* get_last_code() {
    return list_entry(code.prev, InterCode, list);
}

inline bool last_is_temp_assign_cons(Operand *temp) {
    InterCode *last_ic = get_last_code();
    return (last_ic->kind == IR_ASSIGN &&
            last_ic->u.assign.left == temp &&
            last_ic->u.assign.right->kind == CONSTANT);
}

inline bool last_is_temp_assign_vari(Operand *temp) {
    InterCode *last_ic = get_last_code();
    return (last_ic->kind == IR_ASSIGN &&
            last_ic->u.assign.left == temp &&
            last_ic->u.assign.right->kind == VARIABLE);
}

inline bool last_is_read_temp(Operand *temp) {
    InterCode *last_ic = get_last_code();
    return (last_ic->kind == IR_READ &&
            last_ic->u.one.op == temp);
}

void delete_code(InterCode *c) {
    list_del(&c->list);
}

void delete_last_code() {
    delete_code(get_last_code());
}

bool test_ifgoto(Operand *t1, Operand *t2, relop_kind rk) {
    if(t1->kind == CONSTANT && t2->kind == CONSTANT) {
        if(rk == EQ)
        return t1->u.cons == t2->u.cons;
        else if(rk == NE)
        return t1->u.cons != t2->u.cons;
        else if(rk == LT)
            return t1->u.cons < t2->u.cons;
        else if(rk == LE)
            return t1->u.cons <= t2->u.cons;
        else if(rk == GT)
            return t1->u.cons > t2->u.cons;
        else if(rk == GE)
            return t1->u.cons >= t2->u.cons;
    }
    return false;
}

void print_relop(relop_kind rk, FILE *fp) {
    switch(rk) {
        case EQ:
            fprintf(fp, "==");
            break;
        case NE:
            fprintf(fp, "!=");
            break;
        case LT:
            fprintf(fp, "<");
            break;
        case GT:
            fprintf(fp, ">");
            break;
        case LE:
            fprintf(fp, "<=");
            break;
        case GE:
            fprintf(fp, ">=");
            break;
        default: assert(0);
    }
}

void print_code(list_head *code, FILE *fp) {
    list_head *p;
    InterCode *ic;
    list_foreach(p, code) {
        ic = list_entry(p, InterCode, list);
        switch(ic->kind) {
            case IR_ASSIGN:
                print_op(ic->u.assign.left, fp);
                fprintf(fp, " := ");
                print_op(ic->u.assign.right, fp);
                break;
            case IR_ADD:
            case IR_SUB:
            case IR_MUL:
            case IR_DIV:
                print_op(ic->u.binop.result, fp);
                fprintf(fp, " := ");
                print_op(ic->u.binop.op1, fp);
                if(ic->kind == IR_ADD)
                    fprintf(fp, " + ");
                if(ic->kind == IR_SUB)
                    fprintf(fp, " - ");
                if(ic->kind == IR_MUL)
                    fprintf(fp, " * ");
                if(ic->kind == IR_DIV)
                    fprintf(fp, " / ");
                print_op(ic->u.binop.op2, fp);
                break;
            case IR_RETURN:
                fprintf(fp, "RETURN  ");
                print_op(ic->u.one.op, fp);
                break;
            case IR_LABEL:
                fprintf(fp, "LABEL ");
                print_op(ic->u.one.op, fp);
                fprintf(fp, " :");
                break;
            case IR_GOTO:
                fprintf(fp, "GOTO ");
                print_op(ic->u.one.op, fp);
                break;
            case IR_IFGOTO:
                fprintf(fp, "IF ");
                print_op(ic->u.triop.t1, fp);
                fprintf(fp, " ");
                print_relop(ic->u.triop.relop, fp);
                fprintf(fp, " ");
                print_op(ic->u.triop.t2, fp);
                fprintf(fp, " GOTO ");
                print_op(ic->u.triop.label, fp);
                break;
            case IR_READ:
                fprintf(fp, "READ ");
                print_op(ic->u.one.op, fp);
                break;
            case IR_WRITE:
                fprintf(fp, "WRITE ");
                print_op(ic->u.one.op, fp);
                break;
            case IR_CALL:
                print_op(ic->u.assign.left, fp);
                fprintf(fp, " := CALL ");
                print_op(ic->u.assign.right, fp);
                break;
            case IR_ARG:
                fprintf(fp, "ARG ");
                print_op(ic->u.one.op, fp);
                break;
            case IR_FUNCTION:
                fprintf(fp, "FUNCTION ");
                print_op(ic->u.one.op, fp);
                fprintf(fp, " :");
                break;
            case IR_PARAM:
                fprintf(fp, "PARAM ");
                print_op(ic->u.one.op, fp);
                break;
            case IR_DEC:
                fprintf(fp, "DEC ");
                print_op(ic->u.dec.op, fp);
                fprintf(fp, " %d", ic->u.dec.size);
                break;
            case IR_RIGHTAT:
                print_op(ic->u.assign.left, fp);
                fprintf(fp, " := &");
                print_op(ic->u.assign.right, fp);
                break;
            case IR_LEFTSTAR:
                fprintf(fp, "*");
                print_op(ic->u.assign.left, fp);
                fprintf(fp, " := ");
                print_op(ic->u.assign.right, fp);
                break;
            case IR_RIGHTSTAR:
                print_op(ic->u.assign.left, fp);
                fprintf(fp, " := *");
                print_op(ic->u.assign.right, fp);
                break;
            default: assert(0);
        }
        fprintf(fp, "\n");
    }
}

void print_op(Operand *op, FILE *fp) {
    if(!op) return;
    switch(op->kind) {
        case VARIABLE:
            fprintf(fp, "v%d", op->u.var_no);
            break;
        case CONSTANT:
            fprintf(fp, "#%d", op->u.cons);
            break;
        case TEMPVAR:
            fprintf(fp, "t%d", op->u.var_no);
            break;
        case LABEL:
            fprintf(fp, "label%d", op->u.label->no);
            break;
        case FUNCTION:
            fprintf(fp, "%s", op->u.value);
            break;
        default: assert(0);
    }
}

int gen_no(op_kind op) {
    static int t = 1, l = 1, v = 1;
    switch(op) {
        case VARIABLE:
            return v ++;
        case TEMPVAR:
            return t ++;
        case LABEL:
            return l ++;
        default:
            return -1;
    }
}

Operand* new_label() {
    Operand *nl = new(Operand);
    nl->kind = LABEL;
    nl->u.label = new(Label);
    nl->u.label->no = gen_no(LABEL);
    nl->u.label->next = NULL;
    nl->next = NULL;
    return nl;
}

Operand* new_op_var() {
    Operand *v = new(Operand);
    v->kind = VARIABLE;
    v->u.var_no = gen_no(VARIABLE);
    v->next = NULL;
    return v;
}

Operand* new_op_tvar() { Operand *tv = new(Operand);
    tv->kind = TEMPVAR;
    tv->u.var_no = gen_no(TEMPVAR);
    tv->next = NULL;
    return tv;
}

Operand* new_op_func(char *fname) {
    Operand *f = new(Operand);
    f->kind = FUNCTION;
    f->u.value = fname;
    f->next = NULL;
    return f;
}

Operand* new_op_cons(int cons) {
    Operand *c = new(Operand);
    c->kind = CONSTANT;
    c->u.cons = cons;
    c->next = NULL;
    return c;
}

InterCode* new_ic(ir_kind irk) {
    InterCode *ic = new(InterCode);
    ic->kind = irk;
    list_init(&ic->list);
    return ic;
}

static type_t *var_type = NULL;
static bool comp_left = false;

void export_args(Operand *args) {
    if(args->next) export_args(args->next);
    export_code(new(InterCode, IR_ARG, .u.one.op = args));
}

void translate_exp(tnode *exp, Operand *place) {
    assert(label_equal(exp, Exp));
    symbol *s;
    switch(label_of(exp->son)) {
        case _INT_: {
            Operand *op = new_op_cons(exp->son->intval);
            if(!place) break;
            InterCode *ic = new_ic(IR_ASSIGN);
            ic->u.assign.left = place;
            ic->u.assign.right = op;
            export_code(ic);
        }
            break;
        case _LP_:
            translate_exp(tnode_sec_son(exp), place);
            break;
        case _ID_:
            s = find_by_name_global(id_str(exp->son));
            if(label_equal(exp->last_son, ID)) {
                assert(s->op);
                var_type = s->vmes->type;
                if(!place) break;
                InterCode *ic = new_ic(IR_ASSIGN);
                ic->u.assign.left = place;
                ic->u.assign.right = s->op;
                export_code(ic);
            } else {

                Operand *op = new_op_func(s->name);
                if(label_equal(tnode_thi_son(exp), Args)) {
                    Operand *args = translate_args(tnode_thi_son(exp));
                    if(strcmp(s->name, "write") == 0) {
                        if(last_is_temp_assign_cons(args) ||
                                last_is_temp_assign_vari(args)) {
                            args = get_last_code()->u.assign.right;
                            delete_last_code();
                        }
                        InterCode *ic = new(InterCode, IR_WRITE,
                                .u.one.op = args);
                        export_code(ic);
                    } else {
                        InterCode *ic;
                        export_args(args);
                        if(!place) place = new_temp();
                        ic = new(InterCode, IR_CALL,
                                .u.assign.left = place,
                                .u.assign.right = op);
                        export_code(ic);
                    }
                } else {
                    if(strcmp(s->name, "read") == 0) {
                        if(!place) break;
                        InterCode *ic = new(InterCode, IR_READ,
                            .u.one.op = place);
                        export_code(ic);
                    } else {
                        if(!place) place = new_temp();
                        InterCode *ic = new(InterCode, IR_CALL,
                                .u.assign.left = place,
                                .u.assign.right = op);
                        export_code(ic);
                    }
                }
            }
            break;
        case _MINUS_: {
            Operand *t = new_temp();
            translate_exp(exp->last_son, t);
            if(last_is_temp_assign_cons(t)) {
                InterCode *last_ic = get_last_code();
                delete_last_code();
                last_ic->u.assign.right->u.cons =
                    -last_ic->u.assign.right->u.cons;
                if(!place) break;
                export_code(new(InterCode, IR_ASSIGN,
                            .u.assign.left = place,
                            .u.assign.right = last_ic->u.assign.right));
            } else {
                InterCode *ic = new_ic(IR_SUB);
                if(!place) break;
                ic->u.binop.result = place;
                ic->u.binop.op1 = &zero;
                ic->u.binop.op2 = t;
                export_code(ic);
            }
        }
            break;
        case _Exp_: {
            switch(label_of(tnode_sec_son(exp))) {
                case _ASSIGNOP_: {
                    if(label_equal(exp->son->son, ID)) {
                        s = find_by_name_global(id_str(exp->son->son));
                        if(!s->op) s->op = new_op_var();
                        Operand *t = new_temp();
                        translate_exp(exp->last_son, s->op);
                        InterCode *ic = new_ic(IR_ASSIGN);
                        /*
                        InterCode *last_ic = get_last_code();
                        ic->u.assign.left = s->op;
                        if(last_ic->kind == IR_ASSIGN &&
                                last_ic->u.assign.left == t &&
                                last_ic->u.assign.right->kind == CONSTANT) {
                            ic->u.assign.right = last_ic->u.assign.right;
                            delete_code(last_ic);
                        } else
                            ic->u.assign.right = t;
                        export_code(ic);
                        */
                        if(!place) break;
                        ic = new_ic(IR_ASSIGN);
                        ic->u.assign.left = place;
                        ic->u.assign.right = s->op;
                        export_code(ic);
                    } else {
                        Operand *t1 = new_temp();
                        Operand *t2 = new_temp();
                        comp_left = true;
                        translate_exp(exp->son, t1);
                        comp_left = false;
                        InterCode *last_ic = get_last_code();
                        if(last_ic->kind == IR_ADD &&
                                last_ic->u.binop.result == t1 &&
                                last_ic->u.binop.op2 == &zero) {
                            delete_last_code();
                            t1 = last_ic->u.binop.op1;
                        }

                        translate_exp(exp->last_son, t2);
                        if(last_is_temp_assign_cons(t2) ||
                                last_is_temp_assign_vari(t2)) {
                            t2 = get_last_code()->u.assign.right;
                            delete_last_code();
                        }
                        export_code(new(InterCode, IR_LEFTSTAR, .u.assign.left = t1,
                                    .u.assign.right = t2));
                    }
                }
                    break;
                case _PLUS_:
                case _MINUS_:
                case _STAR_:
                case _DIV_: {
                    Operand *t1 = new_temp();
                    Operand *t2 = new_temp();
                    translate_exp(exp->son, t1);
                    if(last_is_temp_assign_cons(t1)) {
                        InterCode *last_ic = get_last_code();
                        delete_last_code();
                        if(label_equal(tnode_sec_son(exp), PLUS) ||
                                label_equal(tnode_sec_son(exp), MINUS)) {
                            if(last_ic->u.assign.right->u.cons == 0) {
                                translate_exp(exp->last_son, t2);
                                export_code(new(InterCode, IR_ASSIGN,
                                            .u.assign.left = place,
                                            .u.assign.right = t2));
                                break;
                            }
                        } else if(label_equal(tnode_sec_son(exp), STAR)) {
                            if(last_ic->u.assign.right->u.cons == 1) {
                                translate_exp(exp->last_son, t2);
                                export_code(new(InterCode, IR_ASSIGN,
                                            .u.assign.left = place,
                                            .u.assign.right = t2));
                                break;
                            }
                        }
                        t1 = last_ic->u.assign.right;
                    }
                    if(last_is_temp_assign_vari(t1)) {
                        t1 = get_last_code()->u.assign.right;
                        delete_last_code();
                    }
                    translate_exp(exp->last_son, t2);
                    if(last_is_temp_assign_cons(t2)) {
                        InterCode *last_ic = get_last_code();
                        delete_last_code();
                        if(label_equal(tnode_sec_son(exp), PLUS) ||
                                label_equal(tnode_sec_son(exp), MINUS)) {
                            if(last_ic->u.assign.right->u.cons == 0) {
                                export_code(new(InterCode, IR_ASSIGN,
                                            .u.assign.left = place,
                                            .u.assign.right = t1));
                                break;
                            }
                        } else if(label_equal(tnode_sec_son(exp), STAR)) {
                            if(last_ic->u.assign.right->u.cons == 1) {
                                export_code(new(InterCode, IR_ASSIGN,
                                            .u.assign.left = place,
                                            .u.assign.right = t1));
                                break;
                            }
                        }
                        t2 = last_ic->u.assign.right;
                    }
                    if(last_is_temp_assign_vari(t2)) {
                        t2 = get_last_code()->u.assign.right;
                        delete_last_code();
                    }
                    InterCode *ic;
                    if(label_equal(tnode_sec_son(exp), PLUS))
                        ic = new_ic(IR_ADD);
                    else if(label_equal(tnode_sec_son(exp), MINUS))
                        ic = new_ic(IR_SUB);
                    else if(label_equal(tnode_sec_son(exp), STAR))
                        ic = new_ic(IR_MUL);
                    else ic = new_ic(IR_DIV);
                    if(!place) break;
                    ic->u.binop.result = place;
                    ic->u.binop.op1 = t1;
                    ic->u.binop.op2 = t2;
                    export_code(ic);
                }
                    break;
                case _RELOP_:
                case _AND_:
                case  _OR_: {
                    Operand *label1 = new_label();
                    if(!place) break;
                    InterCode *ic = new(InterCode, IR_ASSIGN,
                            .u.assign.left = place, .u.assign.right = &zero);
                    export_code(ic);
                    translate_cond(exp, &fall, label1);
                    if(!place) break;
                    ic = new(InterCode, IR_ASSIGN,
                            .u.assign.left = place, .u.assign.right = &one);
                    export_code(ic);
                }
                    break;
                case _LB_: {
                    bool array_in_left = comp_left;
                    Operand *base;
                    //Operand *t1 = new_temp();
                    comp_left = true;
                    translate_exp(exp->son, place);
                    comp_left = false;
                    if(last_is_temp_assign_vari(place)) {
                        base = get_last_code()->u.assign.right;
                        delete_last_code();
                    } else base = place;
                    type_t *var_type_copy = var_type;
                    Operand *type_size = new_op_cons(var_type_copy->ta->tsize);
                    Operand *t2 = new_temp();
                    translate_exp(tnode_thi_son(exp), t2);
                    if(last_is_temp_assign_cons(t2)) {
                        t2 = get_last_code()->u.assign.right;
                        delete_last_code();
                        t2->u.cons = t2->u.cons * type_size->u.cons;
                        export_code(new(InterCode, IR_ADD,
                                    .u.binop.result = place, .u.binop.op1 = base,
                                    .u.binop.op2 = t2));
                    } else {
                        export_code(new(InterCode, IR_MUL, .u.binop.result = t2,
                                    .u.binop.op1 = t2, .u.binop.op2 = type_size));
                        export_code(new(InterCode, IR_ADD,
                                    .u.binop.result = place, .u.binop.op1 = base,
                                    .u.binop.op2 = t2));
                    }
                    if(!place) break;
                    if(array_in_left) {
                        /*
                        export_code(new(InterCode, IR_ASSIGN, .u.assign.left = place,
                                    .u.assign.right = place));
                                    */
                    } else {
                        export_code(new(InterCode, IR_RIGHTSTAR, .u.assign.left = place,
                                    .u.assign.right = place));
                    }
                    var_type = var_type_copy->ta;
                }
                    break;
                case _DOT_: {
                    bool struct_in_left = comp_left;
                    Operand *base;
                    //Operand *t1 = new_temp();
                    comp_left = true;
                    translate_exp(exp->son, place);
                    comp_left = false;
                    if(last_is_temp_assign_vari(place)) {
                        base = get_last_code()->u.assign.right;
                        delete_last_code();
                    } else base = place;

                    type_t *struct_type = var_type;
                    /* find symbol in struct */
                    list_head *p;
                    symbol *s = NULL;
                    list_foreach(p, &struct_type->struct_domain_symbol) {
                        s = list_entry(p, symbol, list);
                        if(strcmp(s->name, id_str(exp->last_son)) == 0)
                            break;
                    }
                    Operand *t2 = new_op_cons(s->offset);
                    export_code(new(InterCode, IR_ADD, .u.binop.result = place,
                                .u.binop.op1 = base, .u.binop.op2 = t2));
                    if(!place) break;
                    if(struct_in_left) {
                        /*
                        export_code(new(InterCode, IR_ASSIGN, .u.assign.left = place,
                                    .u.assign.right = place));
                                    */
                    } else {
                        export_code(new(InterCode, IR_RIGHTSTAR, .u.assign.left = place,
                                    .u.assign.right = place));
                    }
                    var_type = s->vmes->type;
                }
                    break;
                default: break;
            }
        }
            break;
        case _NOT_: {
            Operand *label1 = new_label();
            if(!place) break;
            InterCode *ic = new(InterCode, IR_ASSIGN,
                    .u.assign.left = place, .u.assign.right = &zero);
            export_code(ic);
            translate_cond(exp, &fall, label1);
            if(!place) break;
            ic = new(InterCode, IR_ASSIGN,
                    .u.assign.left = place, .u.assign.right = &one);
            export_code(ic);
        }
            break;
        default: break;
    }
}

void translate_cond(tnode *exp, Operand *label_true, Operand *label_false) {
    assert(label_equal(exp, Exp));
    switch(label_of(exp->son)) {
        case _NOT_: {
            translate_cond(exp->son, label_false, label_true);
        }
            break;
        case _LP_: {
            translate_cond(tnode_sec_son(exp), label_true, label_false);
        }
            break;
        case _Exp_: {
            switch(label_of(tnode_sec_son(exp))) {
                case _RELOP_: {
                    Operand *t1 = new_temp();
                    Operand *t2 = new_temp();
                    translate_exp(exp->son, t1);
                    if(last_is_temp_assign_cons(t1) ||
                            last_is_temp_assign_vari(t1)) {
                        t1 = get_last_code()->u.assign.right;
                        delete_last_code();
                    }
                    translate_exp(exp->last_son, t2);
                    if(last_is_temp_assign_cons(t2) ||
                            last_is_temp_assign_vari(t2)) {
                        t2 = get_last_code()->u.assign.right;
                        delete_last_code();
                    }
                    if(label_true != &fall && label_false != &fall) {
                        InterCode *ic = new(InterCode, IR_IFGOTO,
                                .u.triop.t1 = t1, .u.triop.t2 = t2,
                                .u.triop.relop = tnode_sec_son(exp)->rk,
                                .u.triop.label = label_true);
                        if(test_ifgoto(t1, t2, tnode_sec_son(exp)->rk)) {
                            export_code(new(InterCode, IR_GOTO, .u.one.op = label_true));
                            break;
                        } else if (t1->kind != CONSTANT || t2->kind != CONSTANT)
                            export_code(ic);
                        ic = new(InterCode, IR_GOTO, .u.one.op = label_false);
                        export_code(ic);
                    } else if(label_true != &fall) {
                        if(test_ifgoto(t1, t2, tnode_sec_son(exp)->rk)) {
                            export_code(new(InterCode, IR_GOTO, .u.one.op = label_true));
                        } else
                            export_code(new(InterCode, IR_IFGOTO,
                                        .u.triop.t1 = t1, .u.triop.t2 = t2,
                                        .u.triop.relop = tnode_sec_son(exp)->rk,
                                        .u.triop.label = label_true));
                    } else if(label_false != &fall) {
                        if(test_ifgoto(t1, t2, reverse_relop(tnode_sec_son(exp)->rk)))
                            export_code(new(InterCode, IR_GOTO, .u.one.op = label_false));
                        else
                            export_code(new(InterCode, IR_IFGOTO,
                                        .u.triop.t1 = t1, .u.triop.t2 = t2,
                                        .u.triop.relop = reverse_relop(tnode_sec_son(exp)->rk),
                                        .u.triop.label = label_false));
                    }
                }
                    break;
                case _AND_: {
                    Operand *t;
                    if(label_false != &fall)
                        translate_cond(exp->son, &fall, label_false);
                    else  {
                        t = new_label();
                        translate_cond(exp->son, &fall, t);
                    }
                    translate_cond(exp->last_son, label_true, label_false);
                    if(label_false == &fall)
                        export_code(new(InterCode, IR_LABEL, .u.one.op = t));
                }
                    break;
                case _OR_: {
                    Operand *t;
                    if(label_true != &fall)
                        translate_cond(exp->son, label_true, &fall);
                    else {
                        t = new_label();
                        translate_cond(exp->son, t, &fall);
                    }
                    translate_cond(exp->last_son, label_true, label_false);
                    if(label_true == &fall)
                        export_code(new(InterCode, IR_LABEL, .u.one.op = t));
                }

                    break;
                default: {
                    Operand *t = new_temp();
                    translate_exp(exp, t);
                    if(last_is_temp_assign_cons(t) ||
                            last_is_temp_assign_vari(t)) {
                        t = get_last_code()->u.assign.right;
                        delete_last_code();
                    }
                    if(label_true != &fall && label_false != &fall) {
                        InterCode *ic = new(InterCode, IR_IFGOTO, .u.triop.t1 = t,
                                .u.triop.relop = NE, .u.triop.t2 = &zero,
                                .u.triop.label = label_true);
                        if(test_ifgoto(t, &zero, tnode_sec_son(exp)->rk)) {
                            export_code(new(InterCode, IR_GOTO, .u.one.op = label_true));
                            break;
                        } else if (t->kind != CONSTANT)
                            export_code(ic);
                        ic = new(InterCode, IR_GOTO, .u.one.op = label_false);
                        export_code(ic);
                    } else if(label_true != &fall) {
                        if(test_ifgoto(t, &zero, NE))
                            export_code(new(InterCode, IR_GOTO, .u.one.op = label_true));
                        else
                            export_code(new(InterCode, IR_IFGOTO,.u.triop.t1 = t,
                                        .u.triop.relop = NE, .u.triop.t2 = &zero,
                                        .u.triop.label = label_true));
                    } else if(label_false != &fall) {
                        if(test_ifgoto(t, &zero, EQ))
                            export_code(new(InterCode, IR_GOTO, .u.one.op = label_false));
                        else
                            export_code(new(InterCode, IR_IFGOTO, .u.triop.t1 = t,
                                        .u.triop.relop = EQ, .u.triop.t2 = &zero,
                                        .u.triop.label = label_false));
                    }
                }
                    break;
            }
        }
            break;
        default: {
            Operand *t = new_temp();
            translate_exp(exp, t);
            if(last_is_temp_assign_cons(t) ||
                    last_is_temp_assign_vari(t)) {
                t = get_last_code()->u.assign.right;
                delete_last_code();
            }
            if(label_true != &fall && label_false != &fall) {
                InterCode *ic = new(InterCode, IR_IFGOTO, .u.triop.t1 = t,
                        .u.triop.relop = NE, .u.triop.t2 = &zero,
                        .u.triop.label = label_true);
                if(test_ifgoto(t, &zero, tnode_sec_son(exp)->rk)) {
                    export_code(new(InterCode, IR_GOTO, .u.one.op = label_true));
                    break;
                } else if (t->kind != CONSTANT)
                    export_code(ic);
                ic = new(InterCode, IR_GOTO, .u.one.op = label_false);
                export_code(ic);
            } else if(label_true != &fall) {
                if(test_ifgoto(t, &zero, NE))
                    export_code(new(InterCode, IR_GOTO, .u.one.op = label_true));
                else
                    export_code(new(InterCode, IR_IFGOTO, .u.triop.t1 = t,
                                .u.triop.relop = NE, .u.triop.t2 = &zero,
                                .u.triop.label = label_true));
            } else if(label_false != &fall) {
                if(test_ifgoto(t, &zero, EQ))
                    export_code(new(InterCode, IR_GOTO, .u.one.op = label_false));
                else
                    export_code(new(InterCode, IR_IFGOTO, .u.triop.t1 = t,
                                .u.triop.relop = EQ, .u.triop.t2 = &zero,
                                .u.triop.label = label_false));
            }
        }
            break;
    }
}

Operand *translate_args(tnode *args) {
    assert(label_equal(args, Args));
    Operand *t = new_temp();
    translate_exp(args->son, t);
    if(last_is_temp_assign_vari(t)) {
        t = get_last_code()->u.assign.right;
        delete_last_code();
    }
    if(label_equal(args->last_son, Args)) {
        t->next = translate_args(args->last_son);
    }
    return t;
}

basic_block broot;

void divide_into_basic_blocks(list_head *code, basic_block *root) {
    list_head *p;
    list_head *begin = code, *end = code;
    list_foreach(p, code) {
        InterCode *ic = list_entry(p, InterCode, list);
    }
}

void optimize_basic_block(basic_block *bc) {

}
