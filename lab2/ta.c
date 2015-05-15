#include "_ta.h"
#include "tree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

type_t std_type_int = {_int_, NULL, NULL, NULL};
type_t std_type_float = {_float_, NULL, NULL, NULL};

list_head sstack_root;

void pserror(int err, int line, char *errinfo,
        ...) {
    fprintf(stderr, "Error type %d at line %d: ", err,
            line);
    char *format, *info;
    va_list arg_ptr;
    va_start(arg_ptr, errinfo);
    vfprintf(stderr, errinfo, arg_ptr);
    va_end(arg_ptr);
    fprintf(stderr, "\n");
}

static unsigned hash_pjw(char *name) {
    unsigned val = 0, i;
    for(; *name; ++ name) {
        val = (val << 2) + *name;
        if((i = val & ~0x3fff)) val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

static void init_parse() {
    list_init(&sstack_root);
    push_sstack();
}

func_mes* new_func(type_t *rett) {
    func_mes *fm = (func_mes *)malloc(sizeof(func_mes));
    fm->rett = rett;
    fm->argc = 0;
    list_init(&fm->argv_list);
    return fm;
}

var_mes* new_var(type_t *type) {
    var_mes *vm = (var_mes *)malloc(sizeof(var_mes));
    vm->type =type;
    return vm;
}

symbol* new_symbol(char *name, bool is_var, int line, void *mes) {
    symbol *s = (symbol *)malloc(sizeof(symbol));
    list_init(&s->list);
    s->name = name;
    s->is_var = is_var;
    s->line = line;
    s->fmes = mes;
    return s;
}

type_t* new_type_struct(char *name) {
    type_t *t = (type_t *)malloc(sizeof(type_t));
    tid_of(t) = _struct_;
    t->name = name;
    list_init(&t->struct_domain_symbol);
    list_init(&t->struct_list);
    return t;
}

type_t* new_type_array(int size) {
    type_t *t = (type_t *)malloc(sizeof(type_t));
    tid_of(t) = _array_;
    t->size = size;
    return t;
}

static hash_t* new_hash_table(int hash) {
    hash_t *ht = (hash_t *)malloc(sizeof(hash_t));
    ht->hash = hash;
    list_init(&ht->hash_list);
    list_init(&ht->symbol_list);
    return ht;
}

static sstack* new_sstack() {
    sstack *sst = (sstack *)malloc(sizeof(sstack));
    list_init(&sst->stack_list);
    list_init(&sst->hash_list);
    list_init(&sst->struct_list);
    return sst;
}

static void link_sstack(sstack *sst) {
    list_add_before(&sstack_root, &sst->stack_list);
}

void push_sstack() {
    sstack *sst = new_sstack();
    link_sstack(sst);
}

void pop_sstack() {
    assert(!list_empty(&sstack_root));
    list_del(sstack_root.prev);
}

static void link_hash_table_to_stack(sstack *sst,  hash_t *ht) {
    list_add_before(&sst->hash_list, &ht->hash_list);
}

hash_t* find_by_hash_in_stack(sstack *sst, int hash) {
    list_head *p;
    hash_t *ret;
    list_foreach(p, &sst->hash_list) {
        ret = list_entry(p, hash_t, hash_list);
        if(ret->hash == hash) return ret;
    }
    return NULL;
}

hash_t* find_by_hash_global(int hash) {
    list_head *ptr;
    hash_t* ret = NULL;
    list_foreach(ptr, &sstack_root) {
        sstack *sst = get_sstack(ptr);
        ret = find_by_hash_in_stack(sst, hash);
        if(ret) break;
    }
    return ret;
}

symbol* find_by_name_in_stack(sstack *sst, char *name) {
    int hash = hash_pjw(name);
    hash_t *ht = find_by_hash_in_stack(sst, hash);
    if(ht == NULL) return NULL;
    list_head *p;
    symbol *s;
    list_foreach(p, &ht->symbol_list) {
        s = list_entry(p, symbol, list);
        if(strcmp(s->name, name) == 0) return s;
    }
    return NULL;
}

symbol* find_by_name_global(char *name) {
    list_head *ptr;
    symbol* ret = NULL;
    list_foreach(ptr, &sstack_root) {
        sstack *sst = get_sstack(ptr);
        ret = find_by_name_in_stack(sst, name);
        if(ret) break;
    }
    return ret;
}

symbol* copy_symbol(symbol *src) {
    symbol *s = (symbol *)malloc(sizeof(symbol));
    memcpy(s, src, sizeof(symbol));
    list_init(&s->list);
    return s;
}

void link_symbol_to_hash_table(symbol *s, hash_t *ht) {
    list_add_before(&ht->symbol_list, &s->list);
}

void export_symbol_to_stack(symbol *s, sstack *sst) {
    int hash = hash_pjw(s->name);
    hash_t *ht = find_by_hash_in_stack(sst, hash);
    if(!ht) ht = new_hash_table(hash);
    link_symbol_to_hash_table(s, ht);
}

void export_symbol_to_func(symbol *s, func_mes *fm) {
    list_add_before(&fm->argv_list, &s->list);
    fm->argc ++;
}

void export_symbol_to_struct(symbol *s, type_t *type) {
    assert(type_struct(type));
    list_add_before(&type->struct_domain_symbol, &s->list);
}

void export_func_arg_to_sstack(func_mes *fm, sstack *sst) {
    list_head *ptr;
    symbol *s;
    assert(list_empty(&sst->hash_list));
    list_foreach(ptr, &fm->argv_list) {
        s = list_entry(ptr, symbol, list);
        export_symbol_to_stack(copy_symbol(s), sst);
    }
}

void export_type_struct(type_t *type) {
    assert(type_struct(type));
    list_add_before(&sstack_top->struct_list, &type->struct_list);
}

type_t *get_struct(char *sname) {
    list_head *p;
    type_t *ret;
    sstack *sst;
    for(sst = sstack_top; ; sst = sstack_prev(sst)) {
        list_foreach(p, &sst->struct_list) {
            ret = list_entry(p, type_t, struct_list);
            if(strcmp(ret->name, sname) == 0) return ret;
        }
        if(sst == sstack_bottom) break;
    }
    return NULL;
}

bool domain_of_struct(type_t *st, char *dname) {
    assert(type_struct(st));
    list_head *p;
    symbol *s;
    list_foreach(p, &st->struct_domain_symbol) {
        s = list_entry(p, symbol, list);
        assert(s->is_var);
        if(strcmp(s->name ,dname) == 0) return true;
        if(type_struct(s->vmes->type) && strcmp(s->name, "$") == 0)
            return domain_of_struct(s->vmes->type, dname);
    }
    return false;
}

bool func_arg_dup(func_mes *fm, char *aname) {
    list_head *ptr;
    symbol *args;
    list_foreach(ptr, &fm->argv_list) {
        args = list_entry(ptr, symbol, list);
        if(strcmp(args->name, aname) == 0) return true;
    }
    return false;
}

bool type_equal(type_t *a, type_t *b) {
    if(tid_of(a) != tid_of(b)) return false;
    if(type_struct(a)) {
        list_head *ap, *bp;
        symbol *sa, *sb;
        for(ap = a->struct_domain_symbol.next, bp = b->struct_domain_symbol.next;
                ap != &a->struct_domain_symbol && bp != &b->struct_domain_symbol;
                ap = ap->next, bp = bp->next) {
            sa = list_entry(ap, symbol, list);
            sb = list_entry(bp, symbol, list);
            assert(sa->is_var && sb->is_var);
            if(!type_equal(sa->vmes->type, sb->vmes->type)) return false;
        }
        if(!(ap == &a->struct_domain_symbol && bp == &b->struct_domain_symbol)) return false;
        return true;
    } else if(type_array(a)) {
        return type_equal(a->ta, b->ta);
    } else return true;
}

bool func_arg_check(func_mes *fm, type_t *type, int mode) { static symbol *arg_s; static int argc, argct;
    switch(mode) {
        case FUNC_ARG_CHECK_INIT:
            argc = 0;
            argct = fm->argc;
            arg_s = list_entry(fm->argv_list.next, symbol, list);
            return false;
        case FUNC_ARG_CHECK_GO:
            argc ++;
            if(argc > argct) return false;
            {
                int ret = type_equal(arg_s->vmes->type, type);
                if(argc < argct) arg_s = list_entry(arg_s->list.next,
                        symbol, list);
                return ret;
            }
        case FUNC_ARG_CHECK_END:
            if(argc == argct) return true;
            else return false;
        default: return false;
    }
}


/* parse part;
 *
 *
 */

type_t* struct_specifier(tnode *t) {
    assert(label_equal(t, StructSpecifier));

    type_t *st = NULL;
    if(label_equal(tnode_sec_son(t), Tag)) {
        tnode *id = tnode_sec_son(t)->son;
        assert(label_equal(id, ID));
        st = get_struct(id_str(id));
        if(!st) pserror(ERR_STR_UNDEF, id->line,
                "Undefined structure \"%s\"", id_str(id));
    } else {
        tnode *opt_tag = t->son;
        tnode *id = opt_tag->son;
        assert(label_equal(opt_tag, OptTag));
        if(id == NULL) {
            st = new_type_struct("$");
        } else {
            assert(label_equal(id, ID));
            if(!get_struct(id_str(id))) {
                if(find_by_name_global(id_str(id)))
                    goto Duplicated;
                else st = new_type_struct(id_str(id));
            } else goto Duplicated;
        }
        def_list_in_struct(tnode_for_son(t), st);
        return st;
Duplicated:
        pserror(ERR_STR_REDEF, id->line,
                "Duplicated name \"%s\"", id_str(id));
        return NULL;
    }
    return st;
}

type_t* specifier(tnode *t) {
    assert(label_equal(t, Specifier));
    type_t *ret = NULL;
    if(label_equal(t->son, TYPE)) {
        tnode *tp = t->son->son;
        if(label_equal(tp, INT))
            ret = &std_type_int;
        else if(label_equal(tp, FLOAT))
            ret = &std_type_float;
    } else {
        ret = struct_specifier(t->son);
    }
    return ret;
}

void fun_dec(tnode *t, type_t *rett) {
    assert(label_equal(t, FunDec));
    tnode *id = t->son;
    symbol *fs = NULL;
    func_mes *fm = new_func(rett);
    if(!find_by_name_global(id_str(id))) {
        pserror(ERR_FUNC_REDEF, id->line,
                "Redefined function \"%s\"", id_str(id));
    }

    tnode *vl = tnode_thi_son(t);
    if(label_equal(vl, VarList)) {
        tnode *pd = vl->son;
        symbol *args = NULL;
        while(label_equal(pd, ParamDec)) {
            type_t *argt = specifier(pd->son);
            if(argt != NULL)
                args = var_dec(pd->last_son, argt);
            if(func_arg_dup(fm, args->name))
                pserror(ERR_VARI_REDEF, pd->line,
                        "Redefined varible \"%s\"", args->name);
            else
                export_symbol_to_func(args, fm);
            vl = vl->last_son;
            pd = vl->son;
        }
    }

    compst(t->brother, rett, fm);
}

static type_t* var_dec_array(tnode *t, type_t *type) {
    assert(label_equal(t, VarDec) && label_equal(t->son, VarDec));
    type_t *at = new_type_array(tnode_thi_son(t)->intval);
    at->ta = type;
    if(label_equal(t->son->son, VarDec)) at = var_dec_array(t->son, at);
    return at;
}

symbol *var_dec(tnode *t, type_t *type) {
    assert(label_equal(t, VarDec));
    symbol *s = NULL;
    var_mes *vm;
    tnode *id = t->son;
    if(label_equal(t->son, ID)) {
        vm = new_var(type);
    } else {
        type = var_dec_array(t, type);
        vm = new_var(type);
        while(!label_equal(id, ID)) id = id->son;
    }
    s = new_symbol(id_str(id), true, t->son->line, vm);

    return s;
}

void ext_dec_list(tnode *t, type_t *type) {
    assert(label_equal(t, ExtDecList));
    symbol *s;
    while(label_equal(t, ExtDecList)) {
        s = var_dec(t->son, type);
        if(find_by_name_global(s->name))
            pserror(ERR_VARI_REDEF, s->line,
                    "Redefined variable \"%s\"", s->name);
        else
            export_symbol(s);

        t = t->last_son;
    }
}

void dec(tnode *t, type_t *type, type_t *st) {
    assert(label_equal(t, Dec));
    symbol *s;
    s = var_dec(t->son, type);
    if(!st) {// func
        if(find_by_name_in_stack_top(s->name)) {
            pserror(ERR_VARI_REDEF, s->line,
                    "Redefined variable\"%s\"", s->name);
            return;
        }
        else export_symbol(s);

        if(label_equal(t->last_son, Exp)) {
            type_t *et = expression(t->last_son);
            if(!type_equal(et, s->vmes->type))
                pserror(ERR_TYPE_NOTEQ, s->line,
                        "Type mismatched for assignment");
        }
    } else {
        if(domain_of_struct(st, s->name)) {
            pserror(ERR_STR_DEF_FAIL, s->line,
                    "Redefined field \"%s\"", s->name);
            return;
        }
        else export_symbol_to_struct(s, st);

        if(label_equal(t->last_son, Exp)) {
            pserror(ERR_STR_DEF_FAIL, s->line,
                    "Not allowed \"=\"");
            return;
        }
    }
}

void dec_list(tnode *t, type_t *type, type_t *st) {
    assert(label_equal(t, DecList));
    while(!label_equal(t, Dec)) {
        dec(t->son, st, type);
        t = t->last_son;
    }
}

void def(tnode *t, type_t *st) {
    assert(label_equal(t, Def));
    type_t *type = specifier(t->son);
    dec_list(tnode_sec_son(t), type, st);
}

void def_list(tnode *t, type_t *st) {
    assert(label_equal(t, DecList));
    while(t->son) {
        def(t->son, st);
        t = t->last_son;
    }
}

void stmt(tnode *t, type_t *rett) {
    assert(label_equal(t, Stmt));
    switch(label_of(t->son)) {
        case _Exp_: {
            expression(t->son);
            break;
        }
        case _CompSt_: {
            compst(t->son, rett, NULL);
            break;
        }
        case _RETURN_: {
            type_t *type = expression(tnode_sec_son(t));
            if(!type_equal(rett, type))
                pserror(ERR_RET_MISMATCH, t->son->line,
                        "Type mismatched for return");
            break;
        }
        case _IF_: {
            expression(tnode_thi_son(t));
            if(tnode_fiv_son(t)->brother)
                stmt(tnode_fiv_son(t), rett);
            stmt(t->last_son, rett);
            break;
        }
        case _WHILE_: {
            expression(tnode_thi_son(t));
            stmt(t->last_son, rett);
            break;
        }
        default: break;
    }
}

void stmt_list(tnode *t, type_t *rett) {
    assert(label_equal(t, StmtList));
    while(t->son) {
        stmt(t->son, rett);
        t = t->last_son;
    }
}

void args(tnode *t, func_mes *fm) {
    assert(label_equal(t, Args));
    if(!fm) func_arg_check_init(fm);
    while(label_equal(t, Args)) {
        type_t *arg_t = expression(t->son);
        func_arg_check_go(arg_t);
        t = t->last_son;
    }
    func_arg_check_end();
}

type_t* expression(tnode *t) {

}

void compst(tnode *t, type_t *rett, func_mes *fm) {
    push_sstack();
    export_func_arg(fm);
    def_list_in_func(tnode_sec_son(t));
    stmt(tnode_thi_son(t), rett);
    pop_sstack();
}

void ext_def(tnode *t) {
    type_t *type = specifier(t->son);
    switch(label_of(tnode_sec_son(t))) {
        case _ExtDecList_:
            ext_dec_list(tnode_sec_son(t), type);
        case _FunDec_:
            fun_dec(t, type);
        case _SEMI_:
        default:
            break;
    }
}

void ext_def_list(tnode *t) {
    while(!t->son) {
        ext_def(t->son);
        t = t->last_son;
    }
}

void main_parse(tnode *tp) {
    assert(label_equal(tp, Program));
    ext_def_list(tp->son);
}


