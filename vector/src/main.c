#include "vector.h"
#include <stdio.h>
#include <stdlib.h>

void print_int(void* elem) {
	int x = *(int*) elem;
	printf("%d ", x);
}

void double_int(size_t i, const void* in, void* out) {
	(void) i;

	int x = *(int*) in;
	*(int*) out = x * 2;
}

int int_is_pair(size_t i, const void* in) {
	(void) i;

	return ((*(int*) in) % 2) == 0;
}

int int_equal(const void* a, const void* b) {
	return (*(int*) a) == (*(int*) b);
}

void print_float(void* elem) {
	float x = *(float*) elem;
	printf("%f ", x);
}

void int_inverse(size_t i, const void* in, void* out) {
	(void) i;

	int x = *(const int*) in;

	if (x == 0) {
		*(float*) out = 0.0f;
		return;
	}

	*(float*) out = 1.0f / (float) x;
}

void destroy_int_ptr(void* elem) {
	void* ptr = *(void**) elem;
	free(ptr);
}

void int_ptr_clone(void* dst, const void* src) {
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
	int x[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	Vector v;
	vector_init(&v, sizeof(int), NULL);
	vector_from_raw(&v, x, 10);
	vector_add_printer(&v, print_int);

	vector_print(&v);

	if (vector_insert(&v, 5, &v.size) < 0) {
		printf("erro ao inserir\n");
	}

	if (vector_remove(&v, 7, NULL) < 0) {
		printf("erro ao remover\n");
	}

	if (vector_swap(&v, 1, 2) < 0) {
		printf("erro ao swap\n");
	}

	vector_print(&v);

	Vector v_doubled;
	vector_init(&v_doubled, v.elem_size, NULL);
	vector_add_printer(&v_doubled, print_int);

	if(vector_map(&v, double_int, &v_doubled) < 0) {
		printf("erro ao mapear\n");
	}

	vector_print(&v_doubled);

	Vector v_filtered;
	vector_init(&v_filtered, v.elem_size, NULL);
	vector_add_printer(&v_filtered, print_int);

	if (vector_filter(&v, int_is_pair, &v_filtered) < 0) {
		printf("erro ao filtrar\n");
	}

	vector_print(&v_filtered);

	size_t index;
	int y = 2;

	if (vector_index(&v_filtered, &y, int_equal, &index) < 0) {
		printf("erro ao index\n");
	}

	printf("index: %ld\n", index);

	Vector v_filtered_and_mapped;
	vector_init(&v_filtered_and_mapped, sizeof(float), NULL);
	vector_add_printer(&v_filtered_and_mapped, print_float);

	if (vector_filter_map(&v, int_is_pair,
	 		int_inverse, &v_filtered_and_mapped) < 0) {
		printf("erro ao filtrar e mapear\n");
	}

	vector_print(&v_filtered_and_mapped);

	int k = 7;
	int* z = vector_find(&v, &k, int_equal);

	if (z) {
		(*z)++;
		vector_print(&v);
	}

	int j;

	if (vector_remove(&v, 6, &j) < 0) {
		printf("erro ao remover");
	}

	Vector g;
	vector_init(&g, sizeof(int*), destroy_int_ptr);

	int* l = malloc(sizeof(int));
	*l = 10;

	vector_push(&g, &l);
	vector_add_clone(&g, int_ptr_clone);

	Vector p;
	vector_init(&p, sizeof(int*), destroy_int_ptr);
	vector_clone(&p, &g);

	int* h;

	if (vector_remove(&g, 0, &h) < 0) {
		printf("erro ao remover");
	}

	printf("%d\n", *h);
	free(h);

	printf("g.size = %ld\n", g.size);
	printf("p.size = %ld\n", p.size);

	int** q = vector_get(&p, 0);
	printf("q = %d\n", **q);

	vector_free(&v);
	vector_free(&v_doubled);
	vector_free(&v_filtered);
	vector_free(&v_filtered_and_mapped);
	vector_free(&g);
	vector_free(&p);

	return 0;
}