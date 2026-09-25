#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

typedef void (*Destructor)(void*);
typedef void (*Clone)(const void* src, void* dst);
typedef void (*Printer)(void*);

typedef struct {
	void* data;
	
	size_t size;
	size_t capacity;
	size_t elem_size;
	
	Destructor destroy;
	Printer print;
	Clone clone;
} Vector;

Vector vector_new(size_t elem_size, Destructor destroy);
void vector_push(Vector* v, const void* element);
void vector_free(Vector* v);

/* ---- operator functions ----*/

void vector_print(const Vector* v);
Vector vector_clone(const Vector* src);

/* ------ */

void vector_realloc(Vector* v, size_t new_capacity);
void vector_insert(Vector* v, size_t index, const void* element);
void vector_remove(Vector* v, size_t index, void* out);
void vector_remove_and_destroy(Vector* v, size_t index);

int vector_swap(Vector* v, size_t i, size_t j);

Vector vector_slice(const Vector* v, size_t begin, size_t end);

void* vector_get(Vector* v, size_t index);
const void* vector_get_const(const Vector* v, size_t index);

#endif