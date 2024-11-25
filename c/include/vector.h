#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Vector {
  size_t length;
  size_t capacity;
  size_t data_size;
  uint8_t *data;
} Vector;

Vector *vector_new(size_t length, size_t data_size);
void vector_initialize(Vector *vector, size_t length, size_t data_size);

void vector_cleanup(Vector *vector);

size_t vector_length(const Vector *vector);
size_t vector_data_size(const Vector *vector);
bool vector_is_empty(const Vector *vector);

void vector_resize(Vector *vector, size_t size);
void vector_shrink_to_fit(Vector *vector);

uint8_t *vector_get_back(const Vector *vector);
uint8_t *vector_get_at(const Vector *vector, size_t position);

bool vector_set_at(const Vector *vector, const uint8_t *value, size_t position);

bool vector_push_back(Vector *vector, const uint8_t *value);
bool vector_insert(Vector *vector, const uint8_t *value, size_t position);

bool vector_pop_back(Vector *vector);
bool vector_erase(Vector *vector, size_t position);
void vector_clear(Vector *vector);

void vector_copy(Vector *src, Vector *dest);
void vector_extend(Vector *src, Vector *dest);

#endif // VECTOR_H
