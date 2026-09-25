#include "vector.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

Vector
vector_new(size_t elem_size, 
		   Destructor destroy) 
{
	// cloner and printer must be set manually
	
	Vector v;

	v.data = NULL;
	v.size = 0;
	v.capacity = 0;
	v.elem_size = elem_size;
	
	v.destroy = destroy;
	v.print = NULL;
	v.clone = NULL;
	
	return v;
}

void vector_realloc(Vector* v, size_t new_capacity) {
	if (!v || v->size >= new_capacity) {
		abort();
	}
	
	void* tmp = realloc(v->data, new_capacity * v->elem_size);
	
	if (!tmp && new_capacity) {
		abort();
	}
	
	v->data = tmp;
	v->capacity = new_capacity;
}

void* vector_get(Vector* v, size_t index) {
	assert(v != NULL && index < v->size);
	
	return (char*) v->data + (index * v->elem_size);
}

const void* vector_get_const(const Vector* v, size_t index) {
	assert(v != NULL && index < v->size);
	
	return (const char*) v->data + (index * v->elem_size);
}

Vector vector_clone(const Vector* src) {
	if (src == NULL) {
		abort();
	}
	
	size_t elem_size = src->elem_size;
	
	Vector dst = vector_new(elem_size, src->destroy);
	dst.clone = src->clone;
	dst.print = src->print;
	
	if (src->capacity == 0) {
		return dst;
	}
	
	vector_realloc(&dst, src->capacity);
	
	dst.size = src->size;
	
	if (src->clone) {
		for (size_t i = 0; i < src->size; i++) {
			const void* src_elem = vector_get_const(
				src,
				i
			);
			
			void* dst_elem = vector_get(&dst, i);
			
			src->clone(src_elem, dst_elem);
		}
	}
	
	else {
		memcpy(
			dst.data,
			src->data,
			src->size * elem_size
		);
	}
	
	return dst;
}

// [begin, end)
Vector vector_slice(const Vector* v, size_t begin, size_t end) {
	assert(begin < v->size && end <= v->size && begin != end);
	
	Vector out = vector_new(v->elem_size, v->destroy);
	out.clone = v->clone;
	out.print = v->print;
	
	vector_realloc(&out, end - begin);
	
	if (v->clone) {
		for (size_t i = begin; i < end; i++) {
			const void* v_elem = vector_get_const(
				v,
				i
			);
			
			void* out_elem = vector_get(&out, i - begin);
			
			v->clone(v_elem, out_elem);
		}		
	}
	
	else {
		memcpy(
			out.data,
			(char*) v->data + begin * v->elem_size,
			(end - begin) * v->elem_size
		);
	}
	
	
	out.size = end - begin;
	
	return out;
}

void vector_push(Vector* v, const void* element) {
	if (!v || !element) {
		abort();
	}

	if (v->size == v->capacity) {
		size_t new_capacity = (v->capacity == 0)
			? 4 
			: v->capacity * 2;
		
		vector_realloc(v, new_capacity);
	}

	void* target = (char*) v->data + (v->size * v->elem_size);
	memcpy(target, element, v->elem_size);

	v->size++;
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

void vector_insert(Vector* v, size_t index, const void* element) {
	assert(v != NULL && index < v->size && element != NULL);

	if (v->size == v->capacity) {
		size_t new_capacity = (v->capacity == 0)
			? 4 
			: v->capacity * 2;
		
		vector_realloc(v, new_capacity);
	}

	vector_shift_right(v, index);

	void* target = (char*) v->data + (index * v->elem_size);
	memcpy(target, element, v->elem_size);

	v->size++;
}

void vector_remove(Vector* v, size_t index, void* out) {
	assert(v != NULL && index < v->size);

	void* element = (char*) v->data	+ index * v->elem_size;

	if (out) {
		if (v->clone) {
			v->clone(element, out);
		} 
		
		else {
			memcpy(out, element, v->elem_size);
		}
	}

	vector_shift_left(v, index);
	v->size--;
}

void vector_remove_and_destroy(Vector* v, size_t index) {
	assert(v != NULL && index < v->size && v->destroy != NULL);

	void* element = (char*) v->data + index * v->elem_size;

	v->destroy(element);
	vector_shift_left(v, index);
	v->size--;
}

void vector_print(const Vector* v) {
	assert(v != NULL && v->print != NULL);

	for (size_t i = 0; i < v->size; i++) {
		void* element = (char*) v->data + i * v->elem_size;
		v->print(element);
	}
}

int vector_swap(Vector* v, size_t i, size_t j) {
	assert(v != NULL && i < v->size && j < v->size);
	
	if (i == j) {
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