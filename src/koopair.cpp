#include "koopair.h"
#include "frame.h"

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
/*
    Abandoned for now;
    might be transformed into value_t2reg
    when implement advanced reg alloc
*/
static std::map<koopa_raw_binary_t, std::string> binary2reg;

void Visit(const koopa_raw_program_t &program);

void Visit(const koopa_raw_slice_t &slice);

void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);

void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value);
void Visit(const koopa_raw_store_t &store);
void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value);
void Visit(const koopa_raw_branch_t &branch);
void Visit(const koopa_raw_jump_t &jump);
void Visit(const koopa_raw_call_t &call, const koopa_raw_value_t &value);

void libkoopa_string2rawprog_and_visit(const char *str){
    koopa_program_t program;

    koopa_error_code_t ret = koopa_parse_from_string(str, &program);
    assert(ret == KOOPA_EC_SUCCESS);

    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);

    std::cerr << "DEBUG: RISCV generation started." << std::endl;
    Visit(raw);
    std::cerr << "DEBUG: RISCV generation ended." << std::endl;

    koopa_delete_raw_program_builder(builder);
}

void Visit(const koopa_raw_program_t &program){
    Visit(program.values);
    std::cout << "\t.text" << std::endl;
    Visit(program.funcs);
}

void Visit(const koopa_raw_slice_t &slice){
    std::cerr << "slice_kind: " << slice.kind << "\t"
            <<"slice_len: " << slice.len << std::endl;
    for(size_t i = 0; i < slice.len; ++i){
        std::cerr << "slice_item" << i << std::endl;
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
    // func->ty, func->params

    assert(func->bbs.len != 0);
    func_alloc_frame(func);
    // frame = &(frames[func->name]);

    std::cout << "\t.globl " << func->name + 1 << std::endl;
    std::cout << func->name + 1 << ": " << std::endl;

    /* Prologue */
    size_t frame_size = map_frame2size[frame];
    if(frame_size > STACK_IMM_NEG_MAX){
        /* TODO : li IMM to t0, add sp with t0 */
        assert(false);
    }
    else{
        std::cout << "\taddi\tsp, sp, " << -(int)frame_size << std::endl;
        if(map_frame2is_with_call[frame]){
            std::cout << "\tsw\tra, " << frame_size - 4 << "(sp)" << std::endl;
        }
    }

    Visit(func->bbs);
}

void Visit(const koopa_raw_basic_block_t &bb){
    // bb->used_by, bb->params
    assert(bb->name != nullptr);
    if(std::string(bb->name) != "%entry")
        std::cout << bb->name + 1 << ":" << std::endl;

    Visit(bb->insts);
}

/* Visit "value", but actually visit "inst" */
void Visit(const koopa_raw_value_t &value){
    std::cerr << "\t" << "value_kind:" << value->kind.tag
            << "\t" << "type_tag:" << (value->ty->tag) << std::endl;
    const auto &kind = value->kind;
    switch (kind.tag)
    {
    case KOOPA_RVT_RETURN:
        Visit(kind.data.ret);
        break;
    case KOOPA_RVT_INTEGER:
        assert(false);
        // Visit(kind.data.integer);
        break;
    case KOOPA_RVT_BINARY:
        Visit(kind.data.binary, value);
        break;
    case KOOPA_RVT_ALLOC:
        std::cerr<<"\tname:"<<value->name
                <<"\ttype_tag:" <<value->ty->tag
                <<"\tpointer_type_tag:"<<value->ty->data.pointer.base->tag
                <<"\tvalue_used_by_len:"<<value->used_by.len<<std::endl;
        // Visit(kind.data)
        break;
    case KOOPA_RVT_STORE:
        Visit(kind.data.store);
        break;
    case KOOPA_RVT_LOAD:
        Visit(kind.data.load, value);
        break;
    case KOOPA_RVT_BRANCH:
        Visit(kind.data.branch);
        break;
    case KOOPA_RVT_JUMP:
        Visit(kind.data.jump);
        break;
    case KOOPA_RVT_CALL:
        Visit(kind.data.call, value);
        /* TODO */
        break;
    default:
        /*TODO: Other raw value types*/
        std::cerr << kind.tag << std::endl;
        assert(false);
        break;
    }
    std::cout << std::endl;
}

void Visit(const koopa_raw_return_t &ret){
    if(ret.value != nullptr){
        if(ret.value->kind.tag == KOOPA_RVT_INTEGER){
            std::cout << "\tli\t" << "a0" << ", ";
            std::cout << ret.value->kind.data.integer.value;
            std::cout << std::endl;
        }
        else{
            // if(ret.value->kind.tag == KOOPA_RVT_BINARY){
            //     std::string reg = binary2reg[ret.value->kind.data.binary];
            //     std::cout << "\tmv\t" << "a0, ";
            //     std::cout << reg << std::endl;
            // }
            assert((*frame).find(ret.value) != (*frame).end());
            int offset_ret = (*frame)[ret.value].offset;
            assert(offset_ret >= 0);
            if(offset_ret > STACK_IMM_POS_MAX){
                /* TODO */
                assert(false);
            }
            else{
                std::cout << "\tlw\t" << "a0" << ", ";
                std::cout << offset_ret << "(sp)" << std::endl;
            }
        }
    }

    /* Epilogue */
    size_t frame_size = map_frame2size[frame];
    if(frame_size > STACK_IMM_POS_MAX){
        /* TODO */
        assert(false);
    }
    else{
        if(map_frame2is_with_call[frame]){
            std::cout << "\tlw\tra, " << frame_size - 4 << "(sp)" << std::endl;
        }
        std::cout << "\taddi\tsp, sp, " << frame_size << std::endl;
    }
    std::cout << "\tret" << std::endl;
}

void Visit(const koopa_raw_integer_t &integer){
    std::cout << "\tli\t" << "t" << register_counter++ << ", ";
    std::cout << integer.value << std::endl;
}

void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value){
    const koopa_raw_binary_op_t &op = binary.op;
    const koopa_raw_value_t &lhs = binary.lhs;
    const koopa_raw_value_t &rhs = binary.rhs;
    std::string reg_lhs, reg_rhs;
    std::string reg_result;

    int register_counter_init = register_counter;

    assert(!(lhs->kind.tag == KOOPA_RVT_INTEGER &&
            rhs->kind.tag == KOOPA_RVT_INTEGER));

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
        // assert(lhs->kind.tag == KOOPA_RVT_BINARY);
        // reg_lhs = binary2reg[lhs->kind.data.binary];
        std::cerr << lhs->kind.tag << std::endl;
        assert((*frame).find(lhs) != (*frame).end());
        reg_lhs = "t" + std::to_string(register_counter++);
        std::cout << "\tlw\t" << reg_lhs << ", ";
        std::cout << (*frame)[lhs].offset << "(sp)" << std::endl;
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
        // assert(rhs->kind.tag == KOOPA_RVT_BINARY);
        // reg_rhs = binary2reg[rhs->kind.data.binary];
        std::cerr << rhs->kind.tag << std::endl;
        assert((*frame).find(rhs) != (*frame).end());
        reg_rhs = "t" + std::to_string(register_counter++);
        std::cout << "\tlw\t" << reg_rhs << ", ";
        std::cout << (*frame)[rhs].offset << "(sp)" << std::endl;
    }

    // /*  TODO:
    //     what if both "x0"?
    //     better policy;
    //     check if the value in reg is no longer needed
    // */
    // // reg_result = ((reg_rhs == "x0") ? reg_lhs : reg_rhs);
    // // binary2reg[binary] = reg_result;
    // reg_result = "t" + std::to_string(register_counter++);
    // binary2reg[binary] = reg_result;
    reg_result = "t" + std::to_string(register_counter_init);

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

    // assert((*frame).find(value) != (*frame).end());
    int offset_result = (*frame)[value].offset;
    assert(offset_result >= 0);
    if(offset_result > STACK_IMM_POS_MAX){
        /* TODO */
        assert(false);
    }
    else{
        std::cout << "\tsw\t" << reg_result << ", ";
        std::cout << offset_result << "(sp)" << std::endl;
    }

    register_counter = register_counter_init;
}

void Visit(const koopa_raw_store_t &store){
    if(store.value->kind.tag == KOOPA_RVT_FUNC_ARG_REF){
        size_t idx = store.value->kind.data.func_arg_ref.index;
        if(idx < 8){
            int offset_dest = (*frame)[store.dest].offset;
            assert(offset_dest >= 0);
            if(offset_dest > STACK_IMM_POS_MAX){
                /* TODO */
                assert(false);
            }
            else{
                std::cout << "\tsw\ta" << idx << ", ";
                std::cout << offset_dest << "(sp)" << std::endl;
            }
        }
        else{
            size_t frame_size = map_frame2size[frame];
            idx -= 8;
            int offset_src = frame_size + idx * 4;
            std::cout << "\tlw\tt" << register_counter << ", ";
            std::cout << offset_src << "(sp)" << std::endl;

            int offset_dest = (*frame)[store.dest].offset;
            assert(offset_dest >= 0);
            if(offset_dest > STACK_IMM_POS_MAX){
                /* TODO */
                assert(false);
            }
            else{
                std::cout << "\tsw\tt" << register_counter << ", ";
                std::cout << offset_dest << "(sp)" << std::endl;
            }
        }
    }
    else{
        if(store.value->kind.tag == KOOPA_RVT_INTEGER){
            Visit(store.value->kind.data.integer);
            register_counter--;
        }
        else{

            std::cerr << store.value->kind.tag << ","
                << store.value->name  << ","
                << store.value->ty->tag << "\n";
            assert((*frame).find(store.value) != (*frame).end());
            int offset_src = (*frame)[store.value].offset;
            assert(offset_src >= 0);
            if(offset_src > STACK_IMM_POS_MAX){
                /* TODO */
                assert(false);
            }
            else{
                std::cout << "\tlw\t" << "t" << register_counter << ", ";
                std::cout << offset_src << "(sp)" << std::endl;
            }
        }

        // assert((*frame).find(store.dest) != (*frame).end());
        int offset_dest = (*frame)[store.dest].offset;
        assert(offset_dest >= 0);
        if(offset_dest > STACK_IMM_POS_MAX){
            /* TODO */
            assert(false);
        }
        else{
            std::cout << "\tsw\t" << "t" << register_counter << ", ";
            std::cout << offset_dest << "(sp)" << std::endl;
        }
    }
}

void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value){
    // assert((*frame).find(load.src) != (*frame).end());
    // assert((*frame).find(value) != (*frame).end());
    int offset_src = (*frame)[load.src].offset;
    int offset_dest = (*frame)[value].offset;

    assert(offset_src >=0 && offset_dest >= 0);

    if(offset_src > STACK_IMM_POS_MAX){
        /* TODO */
        assert(false);
    }
    else{
        std::cout << "\tlw\t" << "t" << register_counter << ", ";
        std::cout << offset_src << "(sp)" << std::endl;
    }

    if(offset_dest > STACK_IMM_POS_MAX){
        /* TODO */
        assert(false);
    }
    else{
        std::cout << "\tsw\t" << "t" << register_counter << ", ";
        std::cout << offset_dest << "(sp)" << std::endl;
    }
}

void Visit(const koopa_raw_branch_t &branch){
    if(branch.cond->kind.tag == KOOPA_RVT_INTEGER){
        std::cout << "\tli\t" << "t" << register_counter << ", ";
        std::cout << branch.cond->kind.data.integer.value << std::endl;
        std::cout << "\tbnez\t" << "t" << register_counter << ", ";
        std::cout << branch.true_bb->name + 1 << std::endl;
    }
    else{
        assert((*frame).find(branch.cond) != (*frame).end());
        int offset_cond = (*frame)[branch.cond].offset;
        if(offset_cond > STACK_IMM_POS_MAX){
            /* TODO */
            assert(false);
        }
        else{
            std::cout << "\tlw\t" << "t" << register_counter << ", ";
            std::cout << offset_cond << "(sp)" << std::endl;
        }
        std::cout << "\tbnez\t" << "t" << register_counter << ", ";
        std::cout << branch.true_bb->name + 1 << std::endl;
    }

    std::cout << "\tj\t" << branch.false_bb->name + 1 << std::endl;
}

void Visit(const koopa_raw_jump_t &jump){
    std::cout << "\tj\t" << jump.target->name + 1 << std::endl;
}

void Visit(const koopa_raw_call_t &call, const koopa_raw_value_t &value){
    uint32_t idx, len;

    assert(call.args.kind == KOOPA_RSIK_VALUE);
    len = call.args.len;
    for(idx = 0; idx < len; ++idx){
        auto param = reinterpret_cast<koopa_raw_value_t>(call.args.buffer[idx]);
        if(idx < 8){
            if(param->kind.tag == KOOPA_RVT_INTEGER){
                /* TODO: save the previous value in a[0-7]? */
                std::cout << "\tli\ta" << idx << ", ";
                std::cout << param->kind.data.integer.value << std::endl;
            }
            else{
                assert((*frame).find(param) != (*frame).end());
                int offset_param = (*frame)[param].offset;
                assert(offset_param >= 0);
                if(offset_param > STACK_IMM_POS_MAX){
                    /* TODO */
                    assert(false);
                }
                else{
                    std::cout << "\tlw\ta" << idx << ", ";
                    std::cout << offset_param << "(sp)" << std::endl;
                }
            }
        }
        else{
            if(param->kind.tag == KOOPA_RVT_INTEGER){
                std::cout << "\tli\tt" << register_counter << ", ";
                std::cout << param->kind.data.integer.value << std::endl;

                int offset_dest = (idx - 8) * 4;
                if(offset_dest > STACK_IMM_POS_MAX){
                    /* TODO */
                    assert(false);
                }
                else{
                    std::cout << "\tsw\tt" << register_counter << ", ";
                    std::cout << offset_dest << "(sp)" << std::endl;
                }
            }
            else{
                assert((*frame).find(param) != (*frame).end());
                int offset_param = (*frame)[param].offset;
                assert(offset_param >= 0);
                if(offset_param > STACK_IMM_POS_MAX){
                    /* TODO */
                    assert(false);
                }
                else{
                    std::cout << "\tlw\tt" << register_counter << ", ";
                    std::cout << offset_param << "(sp)" << std::endl;
                }

                int offset_dest = (idx - 8) * 4;
                if(offset_dest > STACK_IMM_POS_MAX){
                    /* TODO */
                    assert(false);
                }
                else{
                    std::cout << "\tsw\tt" << register_counter << ", ";
                    std::cout << offset_dest << "(sp)" << std::endl;
                }
            }
        }
    }
    std::cout << "\tcall\t" << call.callee->name + 1 << std::endl;

    if(value->ty->tag != KOOPA_RTT_UNIT){
        assert((*frame).find(value) != (*frame).end());
        int offset_dest = (*frame)[value].offset;
        std::cout << "\tsw\ta0" << ", ";
        std::cout << offset_dest << "(sp)" << std::endl;
    }
}
