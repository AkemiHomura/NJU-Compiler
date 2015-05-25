#include "tree.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

tnode* new_node(char *in, int line) {
    tnode *new = malloc(sizeof(tnode));
    new->info = malloc(sizeof(char) * (strlen(in) + 1));
    new->line = line;
    strcpy(new->info, in);
    new->parent = NULL;
    new->son = NULL;
    new->last_son = NULL;
    new->brother = NULL;

    new->snum = 0;
    return new;
}

void link_node(tnode *parent, tnode *son) {
    if(son == NULL) return;
    if(parent->son == NULL) {
        parent->son = parent->last_son = son;
    } else {
        parent->last_son->brother = son;
        parent->last_son = son;
    }
    son->parent = parent;

    parent->snum ++;
}


