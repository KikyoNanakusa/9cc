#include <stdlib.h>

typedef enum {
  TY_INT,
  TY_PTR,
  TY_ARRAY,
  TY_CHAR,
} TypeKind;

typedef struct Type Type;
struct Type {
  TypeKind kind;
  Type *ptr_to;     // if kind is TY_POINTER, base is the type of pointer
  int size;         // sizeof(type)

  size_t array_size;   // if kind is TY_ARRAY, array_size is the size of array
};

Type *type_int();
Type *type_char();
Type *pointer_to(Type *type);
Type *array_of(Type *type, size_t size);

