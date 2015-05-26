#ifndef __TREE_PARSE_H__
#define __TREE_PARSE_H__
#include "list.h"
#include "tree.h"
#include "string.h"
#include "stdio.h"

enum bool {
    true = 1, false = 0
};
typedef enum bool bool;

enum _type_id_ {
    _int_, _float_, _array_, _struct_
};
typedef enum _type_id_ _type_id_;

struct type_t {
    _type_id_ tid;
    union {
        struct {
            char *name;
            list_head struct_domain_symbol;
            list_head struct_list;
        };
        struct {
            int size;
            struct type_t *ta; // for array
        };
    };
};
typedef struct type_t type_t;
#define tid_of(x) ((x)->tid)
#define type_int(x) ((x) != NULL && (x)->tid == _int_)
#define type_float(x) ((x) != NULL && (x)->tid == _float_)
#define type_struct(x) ((x) != NULL && (x)->tid == _struct_)
#define type_array(x) ((x) != NULL && (x)->tid == _array_)

#define type_array_size(x) ((x)->size)
#define type_array_next(x) ((x)->ta)
#define type_struct_name(x) ((x)->name)

struct func_mes {
    type_t *rett;
    int argc;
    list_head argv_list;
    int vis_tag;
};
typedef struct func_mes func_mes;

struct var_mes {
    type_t *type;
};
typedef struct var_mes var_mes;

struct symbol {
    list_head list;
    char *name;
    // var or fun
    bool is_var;
    int line;
    union {
        func_mes *fmes;
        var_mes *vmes;
    };
};
typedef struct symbol symbol;

struct hash_t {
    int hash;
    list_head hash_list;
    list_head symbol_list;
};
typedef struct hash_t hash_t;

struct sstack {
    list_head stack_list;
    list_head hash_list;
    list_head struct_list;
};
typedef struct sstack sstack;

extern list_head sstack_root;

#define ERR_VARI_UNDEF      1
#define ERR_FUNC_UNDEF      2
#define ERR_VARI_REDEF      3
#define ERR_FUNC_REDEF      4
#define ERR_TYPE_NOTEQ      5
#define ERR_CAN_NOT_ASSIGN  6
#define ERR_OP_MISMATCH     7
#define ERR_RET_MISMATCH    8
#define ERR_F_PARA_MISMATCH 9
#define ERR_ARRAY_ACCESS    10
#define ERR_FUNC_ACCESS     11
#define ERR_A_OFF_NOT_INT   12
#define ERR_DOT_ON_NOSTR    13
#define ERR_DOMAIN_FAIL     14
#define ERR_STR_DEF_FAIL    15
#define ERR_STR_REDEF       16
#define ERR_STR_UNDEF       17
#define ERR_FUNC_NODEF      18
#define ERR_FUNC_REDEC      19

void pserror(int err, int line ,char *errinfo, ...);
func_mes* new_func(type_t *rett);
var_mes* new_var(type_t *type);
symbol* new_symbol(char *name, bool is_var, int line, void *mes);
type_t* new_type_struct(char *name);
#define no_name_struct(st) (strcmp((st)->name, "$") == 0)
type_t* new_type_array(int size);
void push_sstack();
void pop_sstack();
#define get_sstack(list_ptr) list_entry(list_ptr, sstack, stack_list)
#define sstack_top (list_entry(sstack_root.next, sstack, stack_list))
#define sstack_bottom (list_entry(sstack_root.prev, sstack, stack_list))
#define sstack_down(x) (list_entry(x->stack_list.next, sstack, stack_list))
#define global_sstack sstack_bottom
hash_t* find_by_hash_in_stack(sstack *sst, int hash);
#define find_by_hash_in_last_stack(hash) find_by_hash_in_stack(sstack_top, hash)
hash_t* find_by_hash_global(int hash);
symbol* find_by_name_in_stack(sstack *sst, char *name);
#define find_by_name_in_stack_top(name) find_by_name_in_stack(sstack_top, name)
symbol* find_by_name_global(char *name);
symbol* copy_symbol(symbol *src);
void link_symbol_to_hash_table(symbol *s, hash_t *ht);
void export_symbol_to_stack(symbol *s, sstack *sst);
#define export_symbol(s) export_symbol_to_stack(s, sstack_top)
void export_symbol_to_func(symbol *s, func_mes *fm);
void export_symbol_to_struct(symbol *s, type_t *type);
void export_func_arg_to_sstack(func_mes *fm, sstack *sst);
#define export_func_arg(fm) export_func_arg_to_sstack(fm, sstack_top)
void export_type_struct(type_t *type);
type_t* get_struct(char *sname);
type_t* get_struct_in_sstack(char *sname, sstack *sst);
#define get_struct_in_sstack_top(sname) get_struct_in_sstack(sname, sstack_top)
bool domain_of_struct(type_t *st, char *dname, type_t **dtype);
bool func_arg_dup(func_mes *fm, char *aname);
bool type_equal(type_t *a, type_t *b);
bool func_arg_check(func_mes *fm, type_t *type, int mode);
void func_def_check();
bool func_equal(func_mes *a, func_mes *b);
#define FUNC_ARG_CHECK_INIT  1
#define FUNC_ARG_CHECK_GO    2
#define FUNC_ARG_CHECK_END   3
#define func_arg_check_init(fm) func_arg_check(fm, NULL, FUNC_ARG_CHECK_INIT)
#define func_arg_check_go(type) func_arg_check(NULL, type, FUNC_ARG_CHECK_GO)
#define func_arg_check_end() func_arg_check(NULL, NULL, FUNC_ARG_CHECK_END)


#define label_of(ptr) ((ptr)->syntax_label)
#define id_str(id) ((id)->strval)
#define label_equal(ptr, label) (label_of(ptr) == _##label##_)
#define tnode_sec_son(x) ((x)->son->brother)
#define tnode_thi_son(x) (tnode_sec_son(x)->brother)
#define tnode_for_son(x) (tnode_thi_son(x)->brother)
#define tnode_fiv_son(x) (tnode_for_son(x)->brother)

type_t* struct_specifier(tnode *t);
type_t* specifier(tnode *t);
void fun_dec(tnode *t, type_t *rett);
symbol* var_dec(tnode *t, type_t *type);
void ext_dec_list(tnode *t, type_t *type);
#define dec_in_func(t, type) dec(t, type, NULL)
#define dec_in_struct(t, type, st) dec(t, type, st)
void dec(tnode *t, type_t *type, type_t *st);
#define dec_list_in_func(t, type) dec_list(t, type, NULL)
#define dec_list_in_struct(t, type, st) dec_list(t, type, st)
void dec_list(tnode *t, type_t *type, type_t *st);
#define def_in_func(t) def(t, NULL)
#define def_in_struct(t, st) def(t, st)
void def(tnode *t, type_t *st);
#define def_list_in_struct(t, st) def_list(t, st)
#define def_list_in_func(t) def_list(t, NULL)
void def_list(tnode *t, type_t *st);
void stmt(tnode *t, type_t *ret_type);
void stmt_list(tnode *t, type_t *ret_type);
void args(tnode *t, func_mes *fm);
type_t* expression(tnode *t);
void compst(tnode *t, type_t *ret_type, func_mes *fm);
void ext_def(tnode *t);
void ext_def_list(tnode *t);
void main_parse(tnode *tp);

#ifdef DEBUG
#define xx \
    printf("xx\n");
#endif

static void print_sstack(sstack *sst) {
    list_head *p, *q;
    list_foreach(p, &sst->hash_list) {
        hash_t *ht = list_entry(p, hash_t, hash_list);
        list_foreach(q, &ht->symbol_list) {
            symbol *s = list_entry(q, symbol, list);
            printf("sstack %s\n", s->name);
        }
        printf("one hash\n");
    }
}
static void sprint_sstack(sstack *sst) {
    list_head *p;
    list_foreach(p, &sst->struct_list) {
        type_t *s = list_entry(p, type_t, struct_list);
        printf("sstack %s\n", s->name);
    }
}
#endif

/* no name struct named "$"
 * no name function named "$"
 * they both won't be exported
 * */
