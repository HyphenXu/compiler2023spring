#include "array.h"

#include <iostream>
#include <cassert>

size_t size_of_raw_type(const koopa_raw_type_t &ty){
    switch (ty->tag)
    {
    case KOOPA_RTT_INT32:
        return SIZE_INT32;
        break;
    case KOOPA_RTT_ARRAY:
        return ty->data.array.len * size_of_raw_type(ty->data.array.base);
        break;
    default:
        std::cerr << ty->tag << "\n";
        assert(false);
        break;
    }
}

void riscv_gen_initializer(koopa_raw_value_t value){
    int i;

    switch (value->kind.tag)
    {
    case KOOPA_RVT_ZERO_INIT:
        std::cout << "\t.zero\t";
        std::cout << size_of_raw_type(value->ty) << std::endl;
        break;
    case KOOPA_RVT_INTEGER:
        std::cout << "\t.word\t" << value->kind.data.integer.value;
        std::cout << std::endl;
        break;
    case KOOPA_RVT_AGGREGATE:
        assert(value->kind.data.aggregate.elems.kind == KOOPA_RSIK_VALUE);
        for(i = 0; i < value->kind.data.aggregate.elems.len; ++i){
            riscv_gen_initializer(
                reinterpret_cast<koopa_raw_value_t>(
                    value->kind.data.aggregate.elems.buffer[i]
                )
            );
        }
        break;
    default:
        /* TODO */
        assert(false);
        break;
    }
}
