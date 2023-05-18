#ifndef ARRAY_H
#define ARRAY_H

#include "koopa.h"

#define SIZE_INT32  4

size_t size_of_raw_type(const koopa_raw_type_t &ty);

void riscv_gen_initializer(koopa_raw_value_t value);

#endif /**< src/array.h */
