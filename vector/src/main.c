#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void print_int(void* elem) {
	int x = *(int*) elem;
	printf("%d ", x);
}

void print_float(void* elem) {
	float x = *(float*) elem;
	printf("%f ", x);
}

void destroy_int_ptr(void* elem) {
	void* ptr = *(void**) elem;
	free(ptr);
}

void int_ptr_clone(const void* src, void* dst) {
	const int* original = *(const int**) src;

	if (!original) {
		*(int**) dst = NULL;
		return;
	}

	int* copy = malloc(sizeof(int));

	if (!copy) {
		*(int**) dst = NULL;
		return;
	}

	*copy = *original;
	*(int**) dst = copy;
}

int main(void) {
	Vector v = vector_new(sizeof(int), NULL);
	
	for (size_t i = 0; i < 10; i++) {
		vector_push(&v, &i);
	}
	
	// [start, end)
	Vector slice = vector_slice(&v, 0, 10);
	
	slice.print = print_int;
	vector_print(&slice);
	
	vector_free(&v);
	vector_free(&slice);

	return 0;
}