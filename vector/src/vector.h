#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

typedef void (*Destructor)(void*);
typedef void (*Clone)(void* dst, const void* src);
typedef void (*Printer)(void*);
typedef void (*MapFn)(size_t index, const void* in, void* out);
typedef int (*FilterFn)(size_t index, const void* in);
typedef int (*EqualFn)(const void* x, const void* y);

typedef struct {
	void* data;
	size_t size;
	size_t capacity;
	size_t elem_size;
	Destructor destroy;
	Printer printer;
	Clone clone;
} Vector;

void vector_init(Vector* v, size_t elem_size, Destructor destroy);
void vector_from_raw(Vector* v, const void* data, size_t data_len);
void vector_push(Vector* v, const void* element);
void vector_free(Vector* v);

/* ---- operator functions ----*/

void vector_add_printer(Vector* v, Printer printer);
void vector_add_clone(Vector* v, Clone clone);
void vector_print(const Vector* v);
int vector_clone(Vector* dst, const Vector* src);

/* ------ */

int vector_empty(const Vector* v);
int vector_realloc(Vector* v, size_t new_capacity);
int vector_insert(Vector* v, size_t index, const void* element);
int vector_remove(Vector* v, size_t index, void* out);
int vector_remove_and_destroy(Vector* v, size_t index);
int vector_swap(Vector* v, size_t i, size_t j);

void* vector_get(const Vector* v, size_t index);
void* vector_get_unchecked(const Vector* v, size_t index);

// iterator functions

int vector_map(const Vector* v, MapFn map, Vector* out);
int vector_filter(const Vector* v, FilterFn filter, Vector* out);

int vector_filter_map
(
	const Vector* v, 
	FilterFn filter, 
	MapFn map, 
	Vector* out
);

int vector_index
(
	const Vector* v, 
	const void* target,
	EqualFn equal,
	size_t* out
);

void* vector_find
(	
	const Vector* v, 
	const void* target,
	EqualFn equal
);

int vector_find_and_copy
(	
	const Vector* v, 
	const void* target,
	EqualFn equal,
	void* out
);

int vector_find_and_remove
(	
	Vector* v, 
	const void* target,
	EqualFn equal,
	void* out
);


#endif