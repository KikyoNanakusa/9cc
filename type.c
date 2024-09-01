#include "type.h"

Type *type_int() {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TY_INT;
  type->size = 4;
  return type;
}

Type *pointer_to(Type *type) {
  Type *pointer = calloc(1, sizeof(Type));
  pointer->kind = TY_PTR;
  pointer->ptr_to = type;
  pointer->size = 8;
  return pointer;
}

Type *array_of(Type *type, size_t size) {
  Type *array = calloc(1, sizeof(Type));
  array->kind = TY_ARRAY;
  array->array_of = type;
  array->array_size = size;
  array->size = type->size * size;
  return array;
}
