#include "koopair.h"

#include <iostream>
#include <cassert>
#include <map>

bool operator<(koopa_raw_binary_t a, koopa_raw_binary_t b){
    return ((uint32_t)a.op == (uint32_t)b.op) ? 
            (((uint64_t)a.lhs == (uint64_t)b.lhs) ?
                ((uint64_t)a.rhs < (uint64_t)b.rhs)
                : ((uint64_t)a.lhs < (uint64_t)b.lhs))
            : ((uint32_t)a.op < (uint32_t)b.op);
}

static int register_counter = 0;
static std::map<koopa_raw_binary_t, int> binary2reg;

void Visit(const koopa_raw_slice_t &slice);

void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);

void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);
void Visit(const koopa_raw_binary_t &binary);

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
    case KOOPA_RVT_BINARY:
        Visit(kind.data.binary);
        break;
    default:
        /*TODO: Other raw value types*/
        std::cout << kind.tag << std::endl;
        assert(false);
        break;
    }
}

void Visit(const koopa_raw_return_t &ret){
    if(ret.value != nullptr){
        if(ret.value->kind.tag == KOOPA_RVT_INTEGER){
            std::cout << "\tli\t" << "a0" << ", ";
            std::cout << ret.value->kind.data.integer.value;
            std::cout << std::endl;
        }
        else{
            if(ret.value->kind.tag == KOOPA_RVT_BINARY){
                int regno = binary2reg[ret.value->kind.data.binary];
                std::cout << "\tmv\t" << "a0, ";
                std::cout << "t" << regno << std::endl;
            }
        }
    }
    std::cout << "\tret" << std::endl;
}

void Visit(const koopa_raw_integer_t &integer){
    // std::cout << "\tli " << "a0, " << integer.value << std::endl;
    std::cout << "\tli\t" << "t" << register_counter++ << ", ";
    std::cout << integer.value << std::endl;
    
}

void Visit(const koopa_raw_binary_t &binary){
    koopa_raw_binary_op_t op = binary.op;
    koopa_raw_value_t lhs = binary.lhs;
    koopa_raw_value_t rhs = binary.rhs;
    switch (op)
    {
    case KOOPA_RBO_EQ:
        if(lhs->kind.tag == KOOPA_RVT_INTEGER &&
                lhs->kind.data.integer.value == 0){
            Visit(rhs);
            std::cout << "\txor\t" << "t" << register_counter - 1 << ", ";
            std::cout << "t" << register_counter - 1 << ", x0" << std::endl;
            std::cout << "\tseqz\t" << "t" << register_counter - 1 << ", ";
            std::cout << "t" << register_counter - 1 << std::endl;
            binary2reg[binary] = register_counter - 1;
        }
        else if(rhs->kind.tag == KOOPA_RVT_INTEGER &&
                rhs->kind.data.integer.value == 0){
            Visit(lhs);
            std::cout << "\txor\t" << "t" << register_counter - 1 << ", ";
            std::cout << "t" << register_counter - 1 << ", x0" << std::endl;
            std::cout << "\tseqz\t" << "t" << register_counter - 1 << ", ";
            std::cout << "t" << register_counter - 1 << std::endl;
            binary2reg[binary] = register_counter - 1;
        }
        else{
            /* TODO */
            assert(false);
        }
        break;
    case KOOPA_RBO_SUB:
        if(rhs->kind.tag == KOOPA_RVT_BINARY){
            int regno = binary2reg[rhs->kind.data.binary];
            std::cout << "\tsub\t" << "t" << register_counter++ << ", ";
            std::cout << "x0, " << "t" << regno << std::endl;
            binary2reg[binary] = register_counter - 1;
        }
        else if(rhs->kind.tag == KOOPA_RVT_INTEGER){
            Visit(rhs);
            std::cout << "\tsub\t" << "t" << register_counter << ", ";
            std::cout << "x0, " <<  "t" << register_counter - 1 << std::endl;
            register_counter++;
            binary2reg[binary] = register_counter - 1;
        }
        break;
    default:
        break;
    }
}