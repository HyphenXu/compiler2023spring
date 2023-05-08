#include "koopair.h"

#include <iostream>
#include <cassert>
#include <map>
#include <string>

bool operator<(koopa_raw_binary_t a, koopa_raw_binary_t b){
    return ((uint32_t)a.op == (uint32_t)b.op) ?
            (((uint64_t)a.lhs == (uint64_t)b.lhs) ?
                ((uint64_t)a.rhs < (uint64_t)b.rhs)
                : ((uint64_t)a.lhs < (uint64_t)b.lhs))
            : ((uint32_t)a.op < (uint32_t)b.op);
}

static int register_counter = 0;
static std::map<koopa_raw_binary_t, std::string> binary2reg;

void Visit(const koopa_raw_program_t &program);

void Visit(const koopa_raw_slice_t &slice);

void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);

void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);
void Visit(const koopa_raw_binary_t &binary);

void libkoopa_string2rawprog_and_visit(const char *str){
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
                std::string reg = binary2reg[ret.value->kind.data.binary];
                std::cout << "\tmv\t" << "a0, ";
                std::cout << reg << std::endl;
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
    std::string reg_lhs, reg_rhs;
    std::string reg_result;

    if(lhs->kind.tag == KOOPA_RVT_INTEGER){
        if(lhs->kind.data.integer.value == 0){
            reg_lhs = "x0";
        }
        else{
            reg_lhs = "t" + std::to_string(register_counter);
            Visit(lhs->kind.data.integer);
        }
    }
    else{
        assert(lhs->kind.tag == KOOPA_RVT_BINARY);
        reg_lhs = binary2reg[lhs->kind.data.binary];
    }
    if(rhs->kind.tag == KOOPA_RVT_INTEGER){
        if(rhs->kind.data.integer.value == 0){
            reg_rhs = "x0";
        }
        else{
            reg_rhs = "t" + std::to_string(register_counter);
            Visit(rhs->kind.data.integer);
        }
    }
    else{
        assert(rhs->kind.tag == KOOPA_RVT_BINARY);
        reg_rhs = binary2reg[rhs->kind.data.binary];
    }

    /*  TODO:
        what if both "x0"?
        better policy;
        check if the value in reg is no longer needed
    */
    // reg_result = ((reg_rhs == "x0") ? reg_lhs : reg_rhs);
    // binary2reg[binary] = reg_result;
    reg_result = "t" + std::to_string(register_counter++);
    binary2reg[binary] = reg_result;

    switch (op)
    {
    case KOOPA_RBO_NOT_EQ:
        if(reg_lhs == "x0"){
            std::cout   << "\tsnez\t" << reg_result << ", " << reg_rhs
                        << std::endl;
        }
        else if(reg_rhs == "x0"){
            std::cout   << "\tsnez\t" << reg_result << ", " << reg_lhs
                        << std::endl;
        }
        else{
            std::cout   << "\txor\t" << reg_result << ", " << reg_lhs
                        << ", " << reg_rhs << std::endl;
            std::cout   << "\tsnez\t" << reg_result << ", " << reg_result
                        << std::endl;
        }
        break;
    case KOOPA_RBO_EQ:
        if(reg_lhs == "x0"){
            std::cout   << "\tseqz\t" << reg_result << ", " << reg_rhs
                        << std::endl;
        }
        else if(reg_rhs == "x0"){
            std::cout   << "\tseqz\t" << reg_result << ", " << reg_lhs
                        << std::endl;
        }
        else{
            std::cout   << "\txor\t" << reg_result << ", " << reg_lhs
                        << ", " << reg_rhs << std::endl;
            std::cout   << "\tseqz\t" << reg_result << ", " << reg_result
                        << std::endl;
        }
        break;
    case KOOPA_RBO_GT:
        std::cout   << "\tslt\t" << reg_result << ", " << reg_rhs << ", "
                    << reg_lhs << std::endl;
        break;
    case KOOPA_RBO_LT:
        std::cout   << "\tslt\t" << reg_result << ", " << reg_lhs << ", "
                    << reg_rhs << std::endl;
        break;
    case KOOPA_RBO_GE:
        std::cout   << "\tslt\t" << reg_result << ", " << reg_lhs << ", "
                    << reg_rhs << std::endl;
        std::cout   << "\txori\t" << reg_result << ", " << reg_result
                    << ", 1" << std::endl;
        break;
    case KOOPA_RBO_LE:
        std::cout   << "\tslt\t" << reg_result << ", " << reg_rhs << ", "
                    << reg_lhs << std::endl;
        std::cout   << "\txori\t" << reg_result << ", " << reg_result
                    << ", 1" << std::endl;
        break;
    case KOOPA_RBO_ADD:
        std::cout   << "\tadd\t" << reg_result << ", "
                    << reg_lhs << ", " << reg_rhs << std::endl;
        break;
    case KOOPA_RBO_SUB:
        std::cout   << "\tsub\t" << reg_result << ", "
                    << reg_lhs << ", " << reg_rhs << std::endl;
        break;
    case KOOPA_RBO_MUL:
        std::cout   << "\tmul\t" << reg_result << ", "
                << reg_lhs << ", " << reg_rhs << std::endl;
        break;
    case KOOPA_RBO_DIV:
        std::cout   << "\tdiv\t" << reg_result << ", "
                    << reg_lhs << ", " << reg_rhs << std::endl;
        break;
    case KOOPA_RBO_MOD:
        std::cout   << "\trem\t" << reg_result << ", "
                    << reg_lhs << ", " << reg_rhs << std::endl;
        break;
    case KOOPA_RBO_AND:
        std::cout   << "\tand\t" << reg_result << ", " << reg_lhs << ", "
                    << reg_rhs << std::endl;
        break;
    case KOOPA_RBO_OR:
        std::cout   << "\tor\t" << reg_result << ", " << reg_lhs << ", "
                    << reg_rhs << std::endl;
        break;
    default:
        std::cout << op << std::endl;
        assert(false);
        break;
    }
}
