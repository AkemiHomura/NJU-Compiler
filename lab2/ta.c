#include "_ta.h"
#include "tree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

type_t std_type_int = {_int_, NULL, NULL, NULL};
type_t std_type_float = {_float_, NULL, NULL, NULL};
#define label_of(ptr) ((ptr)->syntax_label)

list_head sstack_root, tstruct_root;

void pserror(int err, int line, char *state,
        char *errinfo) {
    fprintf(stderr, "Error type %d at line %d: %s \"%s\"\n", err,
            line, state, errinfo);
}

static unsigned hash_pjw(char *name) {
    unsigned val = 0, i;
    for(; *name; ++ name) {
        val = (val << 2) + *name;
        if((i = val & ~0x3fff)) val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

static void init() {
    list_init(&sstack_root);
    list_init(&tstruct_root);
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

hash_t* new_hash_table(int hash) {
    hash_t *ht = (hash_t *)malloc(sizeof(hash_t));
    ht->hash = hash;
    list_init(&ht->hash_list);
    list_init(&ht->symbol_list);
    return ht;
}

sstack* new_sstack() {
    sstack *sst = (sstack *)malloc(sizeof(sstack));
    list_init(&sst->stack_list);
    list_init(&sst->hash_list);
    return sst;
}

void link_sstack(sstack *sst) {
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

void link_hash_table_to_stack(sstack *sst,  hash_t *ht) {
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

void link_symbol_to_hash_table(symbol *s, hash_t *ht) {
    list_add_before(&ht->symbol_list, &s->list);
}

void export_symbol_to_stack(sstack *sst, symbol *s) {
    int hash = hash_pjw(s->name);
    hash_t *ht = find_by_hash_in_stack(sst, hash);
    link_symbol_to_hash_table(s, ht);
}

void export_symbol_to_func(symbol *s, func_mes *fm) {
    list_add_before(&fm->argv_list, &s->list);
}

void export_symbol_to_struct(symbol *s, type_t *type) {
    assert(type_struct(type));
    list_add_before(&type->struct_domain_symbol, &s->list);
}

void export_type_struct(type_t *type) {
    assert(type_struct(type));
    list_add_before(&tstruct_root, &type->struct_list);
}

type_t *get_struct(char *sname) {
    list_head *p;
    type_t *ret;
    list_foreach(p, &tstruct_root) {
        ret = list_entry(p, type_t, struct_list);
        if(strcmp(ret->name, sname) == 0) return ret;
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

#define FUNC_ARG_CHECK_INIT  1
#define FUNC_ARG_CHECK_GO    2
#define FUNC_ARG_CHECK_END   3
#define func_arg_check_init(fm) func_arg_check(fm, NULL, FUNC_ARG_CHECK_INIT)
#define func_arg_check_go(type) func_arg_check(NULL, type, FUNC_ARG_CHECK_GO)
#define func_arg_check_end() func_arg_check(NULL, NULL, FUNC_ARG_CHECK_END)


bool func_arg_check(func_mes *fm, type_t *type, int mode) {
    static symbol *arg_s;
    static int argc, argct;
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

