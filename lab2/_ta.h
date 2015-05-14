#ifndef __TREE_PARSE_H__
#define __TREE_PARSE_H__
#include "list.h"
#include "tree.h"

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
#define tid_of(x) (x)->tid
#define type_int(x) ((x)->tid == _int_)
#define type_float(x) ((x)->tid == _int_)
#define type_struct(x) ((x)->tid == _int_)
#define type_array(x) ((x)->tid == _int_)

#define type_array_size(x) ((x)->size)
#define type_array_next(x) ((x)->ta)
#define type_struct_name(x) ((x)->name)

struct func_mes {
    type_t *rett;
    int argc;
    list_head argv_list;
    //int vis_tag;
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
};
typedef struct sstack sstack;

extern list_head sstack_root;
extern list_head tstruct_root;

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
type_t* new_type_array(int size);
void push_sstack();
void pop_sstack();
#define get_sstack(list_ptr) list_entry(list_ptr, sstack, stack_list)
#define sstack_top (list_entry(stack_root.prev, sstack, stack_list))
#define sstack_bottom (list_entry(stack_root.next, sstack, stack_list))
#define global_sstack sstack_bottom
hash_t* find_by_hash_in_stack(sstack *sst, int hash);
#define find_by_hash_in_last_stack(hash) find_by_hash_in_stack(sstack_top, hash)
#define find_by_hash_global(hash) { \
    list_head *ptr, \
    bool ret = false, \
    list_foreach(ptr, &sstack_root) { \
        sstack *sst = get_sstack(ptr); \
        ret |= find_by_hash_in_stack(sst, hash) \
        if(ret) break; \
    }, \
    ret \
}
symbol* find_by_name_in_stack(sstack *sst, char *name);
#define find_by_name_in_last_stack(name) find_by_name_in_stack(sstack_top, name)
#define find_by_name_global(name) { \
    list_head *ptr, \
    bool ret = false, \
    list_foreach(ptr, &sstack_root) { \
        sstack *sst = get_sstack(ptr); \
        ret |= find_by_name_in_stack(sst, name) \
        if(ret) break; \
    }, \
    ret \
}
void link_symbol_to_hash_table(symbol *s, hash_t *ht);
void export_symbol_to_stack(symbol *s, sstack *sst);
#define export_symbol(s) export_symbol_to_stack(s, sstack_top)
void export_symbol_to_func(symbol *s, func_mes *fm);
void export_symbol_to_struct(symbol *s, type_t *type);
void export_type_struct(type_t *type);
type_t* get_struct(char *sname);
bool domain_of_struct(type_t *st, char *dname);
bool type_equal(type_t *a, type_t *b);
bool func_arg_check(func_mes *fm, type_t *type, int mode);


#define label_of(ptr) ((ptr)->syntax_label)

type_t* struct_specifier(tnode *t);
type_t* specifier(tnode *t);
void fun_dec(tnode *t, type_t *ret_type);
symbol* var_dec(tnode *t, type_t *type);
void ext_dec_list(tnode *t, type_t *type);
void dec(tnode *t, type_t *type);
void dec_list(tnode *t, type_t *type);
void def(tnode *t);
void def_list(tnode *t);
void stmt(tnode *t, type_t *ret_type);
void stmt_list(tnode *t, type_t *ret_type);
void args(tnode *t, func_mes *fm);
type_t* expression(tnode *t);
void compst(tnode *t, type_t *ret_type);
void main_parse(tnode *tp);

extern tnode *root;

#endif

/*
 * if one struct has no name, name = NULL
 * and export it's symbol to higher level
 * */

