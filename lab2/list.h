#ifndef __LIST_H__
#define __LIST_H__

#include <stddef.h>
#include "assert.h"

struct list_head {
	struct list_head *prev, *next;
};
typedef struct list_head list_head;

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

static inline void
list_add(list_head *prev, list_head *next, list_head *data) {
	assert(data != NULL);
	data->prev = prev;
	data->next = next;
	if (prev != NULL) prev->next = data;
	if (next != NULL) next->prev = data;
}

static inline void
list_add_before(list_head *list, list_head *data) {
	assert(list != NULL);
	list_add(list->prev, list, data);
}

static inline void
list_add_after(list_head *list, list_head *data) {
	assert(list != NULL);
	list_add(list, list->next, data);
}

static inline void
list_del(list_head *data) { assert(data != NULL);
	list_head *prev = data->prev;
	list_head *next = data->next;
	if (prev != NULL) prev->next = next;
	if (next != NULL) next->prev = prev;
}

static inline void
list_init(list_head *list) {
	assert(list != NULL);
	list->prev = list->next = list;
}

static inline int
list_empty(list_head *list) {
	assert(list != NULL);
	return list == list->next;
}

#define list_foreach(ptr, head) \
	for ((ptr) = (head)->next; (ptr) != (head); (ptr) = (ptr)->next)

#define list_foreach_rev(ptr, head) \
	for ((ptr) = (head)->prev; (ptr) != (head); (ptr) = (ptr)->prev)
#endif

