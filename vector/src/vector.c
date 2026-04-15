#include "vector.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

void vector_init(Vector* v, size_t elem_size, Destructor destroy) {
	/*
	just the destroyer is passed, but if it is a non-null pointer
	then v->clone must be set
	*/

	v->data = NULL;
	v->size = 0;
	v->capacity = 0;
	v->elem_size = elem_size;
	v->destroy = destroy;
	v->printer = NULL;
	v->clone = NULL;
}

int vector_empty(const Vector* v) {
	return (v->size == 0);
}

int vector_realloc(Vector* v, size_t new_capacity) {
	if (!v || v->size >= new_capacity) {
		return -1;
	}

	v->data = realloc(v->data, new_capacity * v->elem_size);
	v->capacity = new_capacity;

	return 0;
}

void vector_from_raw
(
	Vector* v, 
	const void* data, 
	size_t data_len
) 
{
	/*
	this function must be used to create a new vector,
	not to change one that already exists

	vector_init must be called before calling this function
	*/

	if (!v || v->data) {
		return;
	}

	v->data = malloc(v->elem_size * data_len);
	v->capacity = data_len;

	for (size_t i = 0; i < data_len; i++) {
		void* src = (char*) data + (i * v->elem_size);
		void* dst = (char*) v->data + (i * v->elem_size);
		memcpy(dst, src, v->elem_size);
	}

	v->size = data_len;
}

void vector_push(Vector* v, const void* element) {
	if (!v || !element) {
		return;
	}

	if (v->size == v->capacity) {
		size_t new_capacity = v->capacity == 0 ? 4 : v->capacity * 2;
		vector_realloc(v, new_capacity); // ret value is always 0
	}

	void* target = (char*) v->data + (v->size * v->elem_size);
	memcpy(target, element, v->elem_size);

	v->size++;
}

void* vector_get(const Vector* v, size_t index) {
	if (!v) {
		return NULL;
	}

	if (index >= v->size) {
		return NULL;
	}

	return (char*) v->data + (index * v->elem_size);
}

void* vector_get_unchecked(const Vector* v, size_t index) {
	return (char*) v->data + (index * v->elem_size);	
}

void vector_free(Vector* v) {
	if (!v) {
		return;
	}

	if (v->destroy) {
		for (size_t i = 0; i < v->size; i++) {
			void* elem = (char*) v->data + (i * v->elem_size);
			v->destroy(elem);
		}
	}

	free(v->data);
	v->size = 0;
	v->capacity = 0;
}

static void vector_shift_right(Vector* v, size_t index) {
	// the caller needs to ensure that v and index are valid values

	void* dest = (char*) v->data + (index + 1) * v->elem_size;
	void* src =  (char*) v->data + index * v->elem_size;
	size_t n_bytes = (v->size - index) * v->elem_size;

	memmove(dest, src, n_bytes);
}

static void vector_shift_left(Vector* v, size_t index) {
	// the caller needs to ensure that v and index are valid values

	void* dest = (char*) v->data + index * v->elem_size;
	void* src = (char*) v->data + (index + 1) * v->elem_size;
	size_t n_bytes = (v->size - index - 1) * v->elem_size;

	memmove(dest, src, n_bytes);
}

int vector_insert(Vector* v, size_t index, const void* element) {
	if (!v) {
		return -1;
	}
	
	if (index > v->size) {
		return -1;
	}

	if (v->size == v->capacity) {
		size_t new_capacity = v->capacity == 0 ? 4 : v->capacity * 2;
		vector_realloc(v, new_capacity);
	}

	vector_shift_right(v, index);

	void* target = (char*) v->data + (index * v->elem_size);
	memcpy(target, element, v->elem_size);

	v->size++;
	return 0;
}

int vector_remove(Vector* v, size_t index, void* out) {
	/*
	if out and v->destroy are non-null pointers,
	the caller is now the responsible for destroying out
	*/

	if (!v) {
		return -1;
	}

	if (index >= v->size) {
		return -1;
	}

	if (v->destroy && !v->clone) {
		return -1;
	}

	void* element = (char*) v->data	+ index * v->elem_size;

	if (out) {
		if (v->clone) {
			v->clone(out, element);
		} else {
			memcpy(out, element, v->elem_size);
		}
	}

	if (v->destroy) {
		v->destroy(element);
	}

	vector_shift_left(v, index);
	v->size--;

	return 0;
}

int vector_remove_and_destroy(Vector* v, size_t index) {
	if (!v || !v->destroy || index >= v->size) {
		return -1;
	}

	void* element = (char*) v->data + index * v->elem_size;

	v->destroy(element);
	vector_shift_left(v, index);
	v->size--;

	return 0;
}

void vector_add_printer(Vector* v, Printer printer) {
	if (!v) {
		return;
	}

	v->printer = printer;
}

void vector_add_clone(Vector* v, Clone clone) {
	if (!v) {
		return;
	}

	v->clone = clone;
}

void vector_print(const Vector* v) {
	if (!v) {
		return;
	}

	if (!v->printer) {
		printf("v does not have a printer function\n");
		return;
	}

	for (size_t i = 0; i < v->size; i++) {
		void* element = (char*) v->data + i * v->elem_size;
		v->printer(element);
	}

	printf("\n");
}

int vector_clone(Vector* dst, const Vector* src) {
	if (!src || !dst) {
		return -1;
	}

	if (src->elem_size != dst->elem_size) {
		return -1;
	}

	if (dst->data) {
		vector_free(dst);
	}

	if (vector_realloc(dst, src->capacity) < 0) {
		return -1;
	}

	if (src->clone) {
		for (size_t i = 0; i < src->size; i++) {
			void* src_elem = (char*) src->data + (i * src->elem_size);
			void* dst_elem = (char*) dst->data + (i * dst->elem_size);

			src->clone(dst_elem, src_elem);
			dst->size++;
		}

		dst->printer = src->printer;
		dst->clone = dst->clone;
	} else if (!src->destroy) {
		memcpy(dst->data, src->data, src->size * src->elem_size);

		dst->size = src->size;
		dst->printer = src->printer;
	} else {
		free(dst->data);
		dst->data = NULL;

		return -1;
	}

	return 0;
}

int vector_swap(Vector* v, size_t i, size_t j) {
	if (!v || i >= v->size || j >= v->size || i == j) {
		return -1;
	}

	void* a = (char*) v->data + (i * v->elem_size);
	void* b = (char*) v->data + (j * v->elem_size);

	void* tmp = malloc(v->elem_size);

	if (!tmp) {
		return -1;
	}

	memcpy(tmp, a, v->elem_size);
	memcpy(a, b, v->elem_size);
	memcpy(b, tmp, v->elem_size);

	free(tmp);

	return 0;
}

int vector_map(const Vector* v, MapFn map, Vector* out) {
	/*
	out must be a vector that has just been initialized
	*/

	if (!v || !v->data || !out || out->data) {
		return -1; // an error
	}

	if (v->size == 0) {
		return 1; // not an error
	}

	out->data = malloc(v->size * out->elem_size);

	if (!out->data) {
		return -1;
	}

	out->capacity = v->size;

	for (size_t i = 0; i < v->size; i++) {
		const void* elem = (char*) v->data + (i * v->elem_size);
		void* target = (char*) out->data + (i * out->elem_size);

		map(i, elem, target);
	}

	out->size = v->size;
	return 0;
}

int vector_filter(const Vector* v, FilterFn filter, Vector* out) {
	/*
	out must be a vector that has just been initialized
	out and v must contain elements of the same type
	*/

	if (!v || !v->data || !out || out->data) {
		return -1; // an error
	}

	if (v->size == 0) {
		return 1;
	}

	if (v->elem_size != out->elem_size) {
		return -1;
	}

	for (size_t i = 0; i < v->size; i++) {
		void* elem = (char*) v->data + (i * v->elem_size);

		if (filter(i, elem)) {
			vector_push(out, elem);
		}
	}

	return 0;
}

int vector_filter_map
(
	const Vector* v, 
	FilterFn filter, 
	MapFn map, 
	Vector* out
)
{
	if (!v || !v->data || !out || out->data) {
		return -1; // an error
	}

	if (v->size == 0) {
		return 1;
	}

	size_t j = 0;

	for (size_t i = 0; i < v->size; i++) {
		void* elem = (char*) v->data + (i * v->elem_size);

		if (filter(i, elem)) {
			char tmp[out->elem_size];
			map(i, elem, tmp);

			vector_push(out, tmp);
			j++;
		}		
	}

	return 0;
}

int vector_index
(
	const Vector* v, 
	const void* target, 
	EqualFn equal, 
	size_t* out
) 
{
	if (!v || !v->data || !out || !target) {
		return -1;
	}

	if (v->size == 0) {
		return -1;
	}

	for (size_t i = 0; v->size; i++) {
		void* elem = (char*) v->data + (i * v->elem_size);

		if (equal(elem, target)) {
			*out = i;
			return 0;
		}
	}

	return -1;
}

void* vector_find
(	
	const Vector* v, 
	const void* target,
	EqualFn equal
)
{
	if (!v || !v->data || !target) {
		return NULL;
	}

	if (v->size == 0) {
		return NULL;
	}

	for (size_t i = 0; v->size; i++) {
		void* elem = (char*) v->data + (i * v->elem_size);

		if (equal(elem, target)) {
			return elem;
		}
	}

	return NULL;
}

int vector_find_and_copy
(	
	const Vector* v, 
	const void* target,
	EqualFn equal,
	void* out
)
{
	if (!v || !v->data || !target || !out) {
		return -1; // error
	}

	if (v->size == 0) {
		return 1;
	}

	for (size_t i = 0; v->size; i++) {
		void* elem = (char*) v->data + (i * v->elem_size);

		if (equal(elem, target)) {
			memcpy(out, elem, v->elem_size);

			return 0;
		}
	}

	return 2; // not found	
}

int vector_find_and_remove
(	
	Vector* v, 
	const void* target,
	EqualFn equal,
	void* out
)
{
	if (!v || !v->data || !target || !out) {
		return -1; // error
	}

	if (v->size == 0) {
		return 1;
	}

	for (size_t i = 0; v->size; i++) {
		void* elem = (char*) v->data + (i * v->elem_size);

		if (equal(elem, target)) {
			if (vector_remove(v, i, out) < 0) {
				return -1;
			}

			return 0;
		}
	}

	return 2; // not found
}