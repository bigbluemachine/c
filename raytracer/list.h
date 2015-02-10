#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <string.h>

typedef unsigned char octet;

struct _list {
	unsigned int capacity, count;
	size_t size;
	octet *data;
};
typedef struct _list list;

list *list_make(unsigned int k, size_t s) {
	list *l;
	l = (list *) calloc(1, sizeof(list));
	l->data = (octet *) calloc(l->capacity = k, l->size = s);
	return l;
}

void list_free(list *l) {
	free(l->data);
	free(l);
}

void list_add(list *l, const void *e) {
	octet *ptr;
	if(l->capacity == l->count) {
		ptr = (octet *) calloc(l->capacity <<= 1, l->size);
		memcpy(ptr, l->data, l->count * l->size);
		free(l->data);
		l->data = ptr;
	}
	memcpy(l->data + (l->size * l->count++), e, l->size);
}

void *list_get(list *l, size_t i) {
	return l->data + (i * l->size);
}

#endif
