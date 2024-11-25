#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

const size_t VEC_MIN_SIZE = 4;

Vector *vector_new(size_t length, size_t data_size) {
  Vector *vector = calloc(1, sizeof(Vector));
  if (vector == NULL) {
    return NULL; // Allocation failed
  }
  *vector = (Vector){0};
  vector_initialize(vector, length, data_size);
  return vector;
}

void vector_initialize(Vector *vector, size_t length, size_t data_size) {
  size_t capacity = VEC_MIN_SIZE;
  while (capacity < length) {
    capacity <<= 1;
  }
  *vector = (Vector){.data_size = data_size,
                     .capacity = capacity,
                     .length = length,
                     .data = calloc(capacity, data_size)};
  assert(vector->data != NULL);
}

void vector_cleanup(Vector *vector) {
  if (vector->data) {
    free(vector->data);
    vector->data = NULL; // Avoid dangling pointer
  }
  vector->length = 0;
  vector->capacity = 0;
  vector->data_size = 0; // Reset struct fields
}

size_t vector_length(const Vector *vector) { return vector->length; }
size_t vector_data_size(const Vector *vector) { return vector->data_size; }
bool vector_is_empty(const Vector *vector) { return vector->length == 0; }

void vector_resize(Vector *vector, size_t size) {
  if (size == vector->length) {
    return;
  }

  if (size > vector->capacity) {
    while (vector->capacity < size) {
      vector->capacity <<= 1;
    }
    uint8_t *new_data =
        realloc(vector->data, vector->capacity * vector->data_size);
    if (new_data == NULL) {
      return; // Handle allocation error
    }
    vector->data = new_data;
  }

  if (size > vector->length) {
    memset(&vector->data[vector->length * vector_data_size(vector)], 0,
           (size - vector->length) * vector_data_size(vector));
  }
  vector->length = size;
}

void vector_shrink_to_fit(Vector *vector) {
  if (vector->length < vector->capacity) {
    uint8_t *new_data =
        realloc(vector->data, vector_length(vector) * vector_data_size(vector));
    if (new_data == NULL) {
      return; // Handle allocation error
    }
    vector->data = new_data;
    vector->capacity = vector_length(vector);
  }
}

uint8_t *vector_get_back(const Vector *vector) {
  if (vector_is_empty(vector)) {
    return NULL; // Vector is empty
  }
  return vector_get_at(vector, vector_length(vector) - 1);
}

uint8_t *vector_get_at(const Vector *vector, size_t position) {
  if (position >= vector->length) {
    return NULL; // Out of bounds
  }
  return &vector->data[position * vector->data_size];
}

bool vector_set_at(const Vector *vector, const uint8_t *value,
                   size_t position) {
  if (value == NULL || position > vector_length(vector)) {
    return false; // Invalid parameters
  }
  memcpy(vector_get_at(vector, position), value, vector_data_size(vector));
  return true;
}

bool vector_push_back(Vector *vector, const uint8_t *value) {
  if (value == NULL) {
    return false; // Invalid value
  }
  vector_insert(vector, value, vector_length(vector));
  return true;
}

bool vector_insert(Vector *vector, const uint8_t *value, size_t position) {
  if (value == NULL || position > vector_length(vector)) {
    return false; // Invalid parameters
  }
  vector_resize(vector, vector->length + 1);
  for (size_t pos = vector_length(vector) - 1; pos > position; pos--) {
    memcpy(vector_get_at(vector, pos), vector_get_at(vector, pos - 1),
           vector_data_size(vector));
  }
  memcpy(vector_get_at(vector, position), value, vector_data_size(vector));
  return true;
}

bool vector_pop_back(Vector *vector) {
  if (vector_length(vector) == 0) {
    return false; // Nothing to pop
  }
  vector_resize(vector, vector_length(vector) - 1);
  return true;
}

bool vector_erase(Vector *vector, size_t position) {
  if (position >= vector_length(vector)) {
    return false; // Invalid position
  }
  if (position < vector_length(vector) - 1) {
    memcpy(vector_get_at(vector, position), vector_get_at(vector, position + 1),
           vector_data_size(vector) * (vector_length(vector) - position - 1));
  }
  vector_resize(vector, vector_length(vector) - 1);
  return true;
}

void vector_clear(Vector *vector) { vector_resize(vector, 0); }

void vector_copy(Vector *src, Vector *dest) {
  if (vector_data_size(src) != vector_data_size(dest)) {
    return; // Data sizes must match
  }
  vector_resize(dest, vector_length(src));
  memcpy(dest->data, src->data, vector_data_size(src) * vector_length(src));
}

void vector_extend(Vector *src, Vector *dest) {
  if (vector_data_size(src) != vector_data_size(dest)) {
    return; // Data sizes must match
  }
  size_t old_dest_length = vector_length(dest);
  vector_resize(dest, old_dest_length + vector_length(src));
  memcpy(vector_get_at(dest, old_dest_length), src->data,
         vector_data_size(src) * vector_length(src));
}
