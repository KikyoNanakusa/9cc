#include <stdlib.h>

typedef enum {
  TY_INT,
  TY_PTR,
} TypeKind;

typedef struct Type Type;
struct Type {
  TypeKind kind;
  Type *ptr_to;     // if kind is TY_POINTER, base is the type of pointer
  int size;         // sizeof(type)
};

Type *type_int();
Type *pointer_to(Type *type);

