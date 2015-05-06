#ifndef __TREE_H__
#define __TREE_H__

struct tnode {
    char *info;
    int line;
    union {
        int intval;
        double floval;
        char *strval;
    };
    struct tnode *parent;
    struct tnode *son[7];
    int snum;
};

typedef struct tnode tnode;

#define TERMINALS_LINE -1

tnode* new_node(char *, int);
void link_node(tnode *, tnode *);

#endif
