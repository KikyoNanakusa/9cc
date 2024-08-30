#include "type.h"

Type *type_int() {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TY_INT;
  return type;
}

Type *pointer_to(Type *type) {
  Type *pointer = calloc(1, sizeof(Type));
  pointer->kind = TY_PTR;
  pointer->ptr_to = type;
  return pointer;
}
