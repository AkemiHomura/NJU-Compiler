#ifndef __TREE_ANALYSIS_H__
#define __TREE_ANALYSIS_H__
#include <stdint.h>
#include "list.h"

enum _type {
    _int, _float, _array, _struct, _void
};
typedef enum _type _type;

struct type_t {
    enum _type tname;
    struct {
        struct symbol *sec_lv;
        struct symbol *sam_lv;
    };
    int size;
};
typedef struct type_t type_t;

struct stack_ptr {
    list_head stack_list;
    list_head symbol_list;
};
typedef struct stack_ptr stack_ptr;

struct hash_table {
    list_head hash_list;
    list_head symbol_list;
    int hash;
};
typedef struct hash_table hash_table;

struct func_mes {
    enum _type ret_type;
    int argc;
    type_t** argv;
};
typedef struct func_mes func_mes;

struct var_mes {
    struct type_t *type;
};
typedef struct var_mes var_mes;

struct symbol {
    list_head col_list;
    list_head row_list;
    char *name;
    /* function 0
     * variable 1
     * */
    int func_or_variable;
    int visited_tag;
    int lineno;
    union {
        struct func_mes *function_message;
        struct var_mes *variable_message;
    };
};
typedef struct symbol symbol;

extern list_head stack_root;
extern list_head hash_root;


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


#endif
