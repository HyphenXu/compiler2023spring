#include "koopair.h"

#include <iostream>
#include <cassert>

void Visit(const koopa_raw_slice_t &slice);

void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);

void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);

void libkoopa_string2rawprog(const char *str){
    koopa_program_t program;

    koopa_error_code_t ret = koopa_parse_from_string(str, &program);
    assert(ret == KOOPA_EC_SUCCESS);

    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);

    Visit(raw);

    koopa_delete_raw_program_builder(builder);
}

void Visit(const koopa_raw_program_t &program){
    Visit(program.values);
    std::cout << "\t.text" << std::endl;
    Visit(program.funcs);
}

void Visit(const koopa_raw_slice_t &slice){
    for(size_t i = 0; i < slice.len; ++i){
        auto ptr = slice.buffer[i];
        switch (slice.kind)
        {
        case KOOPA_RSIK_UNKNOWN:
            /*TODO: what is UNKNOWN? */
            assert(false);
            break;
        case KOOPA_RSIK_TYPE:
            /*TODO */
            assert(false);
            break;
        case KOOPA_RSIK_FUNCTION:
            Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
            break;
        case KOOPA_RSIK_VALUE:
            Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
            break;
        default:
            assert(false);
            break;
        }
    }
}

void Visit(const koopa_raw_function_t &func){
    std::cout << "\t.global " << func->name + 1 << std::endl;
    std::cout << func->name + 1 << ": " << std::endl;
    Visit(func->bbs);
}

void Visit(const koopa_raw_basic_block_t &bb){
    Visit(bb->insts);
}

void Visit(const koopa_raw_value_t &value){
    const auto &kind = value->kind;
    switch (kind.tag)
    {
    case KOOPA_RVT_RETURN:
        Visit(kind.data.ret);
        break;
    case KOOPA_RVT_INTEGER:
        Visit(kind.data.integer);
        break;
    default:
        /*TODO: Other raw value types*/
        assert(false);
        break;
    }
}

void Visit(const koopa_raw_return_t &ret){
    if(ret.value != nullptr){
        Visit(ret.value);
    }
    std::cout << "\tret" << std::endl;
}

void Visit(const koopa_raw_integer_t &integer){
    std::cout << "\tli " << "a0, " << integer.value << std::endl;
}