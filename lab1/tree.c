#include "tree.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

tnode* new_node(char *in, int line) {
    tnode *new = malloc(sizeof(tnode));
    new->info = malloc(sizeof(char) * (strlen(in) + 1));
    new->line = line;
    strcpy(new->info, in);
    new->parent = NULL;
    int i = 0;
    for(; i < 7; i ++) new->son[i] = NULL;
    new->snum = 0;

    return new;
}

void link_node(tnode *parent, tnode *son) {
    if(son == NULL) return;
    int i = parent->snum;
    assert(i < 8);
    parent->son[i] = son;
    son->parent = parent;
    parent->snum ++;
}

