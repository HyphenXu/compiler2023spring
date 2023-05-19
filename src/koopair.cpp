#include "koopair.h"
#include "frame.h"
#include "array.h"
#include "riscv.h"

#include <iostream>
#include <cassert>
#include <map>
#include <string>

 frames_t frames;
 frame_t *frame;
 map_frame2size_t map_frame2size;
 map_frame2bool_t map_frame2is_with_call;

 int register_counter = 0;

static std::string rd, rs1, rs2, rs;

static std::map<koopa_raw_value_t, std::string> globl2name;

void Visit(const koopa_raw_program_t &program);

void Visit(const koopa_raw_slice_t &slice);

void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);

void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value);
void Visit(const koopa_raw_store_t &store);
void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value);
void Visit(const koopa_raw_branch_t &branch);
void Visit(const koopa_raw_jump_t &jump);
void Visit(const koopa_raw_call_t &call, const koopa_raw_value_t &value);
void Visit(const koopa_raw_global_alloc_t &globl_alloc, const koopa_raw_value_t &value);
void Visit(const koopa_raw_get_elem_ptr_t &get_elem_ptr, const koopa_raw_value_t &value);

/**
 * @brief string Koopa IR --(libkoopa)--> Koopa raw program --(Visit)--> RISCV
 *
 * @param str string-form Koopa IR
 */
void libkoopa_string2rawprog2riscv(const char *str){
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
    std::cout << "\t.data" << std::endl;
    Visit(program.values);
    std::cout << std::endl;
    std::cout << "\t.text" << std::endl;
    Visit(program.funcs);
}

void Visit(const koopa_raw_slice_t &slice){

std::cerr << "slice_kind: " << slice.kind << "\t" <<"slice_len: " << slice.len << std::endl;

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
    /* ignore decl */
    if(func->bbs.len == 0){
        return;
    }

    func_alloc_frame(func);

    std::cout << "\t.globl " << func->name + 1 << std::endl;
    std::cout << func->name + 1 << ": " << std::endl;

    /* Prologue */
    size_t frame_size = map_frame2size[frame];
    gen_addi("sp", "sp", -(int32_t)frame_size);
    if(map_frame2is_with_call[frame]){
        gen_sw("ra", (int32_t)(frame_size - 4), "sp");
    }

    std::cout << std::endl;

    Visit(func->bbs);
}

void Visit(const koopa_raw_basic_block_t &bb){
    assert(bb->name != nullptr);
    if(std::string(bb->name) != "%entry"){
        std::cout << bb->name + 1 << ":" << std::endl;
    }

    Visit(bb->insts);
}

/* Visit "value", but actually visit "inst" */
void Visit(const koopa_raw_value_t &value){

std::cerr << "\t" << "value_kind:" << value->kind.tag << "\t" << "type_tag:" << (value->ty->tag) << std::endl;

    const auto &kind = value->kind;
    switch (kind.tag)
    {
    case KOOPA_RVT_INTEGER:
        /* Not expecting integer as inst */
        assert(false);
        break;

    case KOOPA_RVT_ALLOC:

std::cerr<<"\tname:"<<value->name <<"\ttype_tag:" <<value->ty->tag <<"\tpointer_type_tag:"<<value->ty->data.pointer.base->tag <<"\tvalue_used_by_len:"<<value->used_by.len<<std::endl;

        /* Already dealt with in prologue */
        break;
    case KOOPA_RVT_GLOBAL_ALLOC:
        /* Global */
        Visit(kind.data.global_alloc, value);
        break;
    case KOOPA_RVT_LOAD:
        Visit(kind.data.load, value);
        break;
    case KOOPA_RVT_STORE:
        /* No LVal */
        Visit(kind.data.store);
        break;
    case KOOPA_RVT_GET_PTR:
        /* TODO */
        assert(false);
        break;
    case KOOPA_RVT_GET_ELEM_PTR:
        Visit(kind.data.get_elem_ptr, value);
        break;
    case KOOPA_RVT_BINARY:
        Visit(kind.data.binary, value);
        break;
    case KOOPA_RVT_BRANCH:
        /* No LVal */
        Visit(kind.data.branch);
        break;
    case KOOPA_RVT_JUMP:
        /* No LVal */
        Visit(kind.data.jump);
        break;
    case KOOPA_RVT_CALL:
        Visit(kind.data.call, value);
        break;
    case KOOPA_RVT_RETURN:
        /* No LVal */
        Visit(kind.data.ret);
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
    /* load return value if necessary */
    if(ret.value != nullptr){
        if(ret.value->kind.tag == KOOPA_RVT_INTEGER){
            gen_li("a0", ret.value->kind.data.integer.value);
        }
        else{
            assert((*frame).find(ret.value) != (*frame).end());
            size_t offset_ret = (*frame)[ret.value].offset;
            gen_lw("a0", (int32_t)offset_ret, "sp");
        }
    }

    /* Epilogue */
    size_t frame_size = map_frame2size[frame];
    if(map_frame2is_with_call[frame]){
        gen_lw("ra", (int32_t)(frame_size - 4), "sp");
    }
    gen_addi("sp", "sp", (int32_t)frame_size);
    gen_ret();
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
            rd = "t" + std::to_string(register_counter++);
            gen_li(rd, lhs->kind.data.integer.value);
        }
    }
    else{
        assert((*frame).find(lhs) != (*frame).end());
        reg_lhs = "t" + std::to_string(register_counter++);
        size_t offset_lhs = (*frame)[lhs].offset;
        gen_lw(reg_lhs, (int32_t)offset_lhs, "sp");
    }

    if(rhs->kind.tag == KOOPA_RVT_INTEGER){
        if(rhs->kind.data.integer.value == 0){
            reg_rhs = "x0";
        }
        else{
            reg_rhs = "t" + std::to_string(register_counter);
            rd = "t" + std::to_string(register_counter++);
            gen_li(rd, rhs->kind.data.integer.value);
        }
    }
    else{
        assert((*frame).find(rhs) != (*frame).end());
        reg_rhs = "t" + std::to_string(register_counter++);
        size_t offset_rhs = (*frame)[rhs].offset;
        gen_lw(reg_rhs, (int32_t)offset_rhs, "sp");
    }

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
        /* TODO what about other binary ops? (e.g. shift?)*/
        assert(false);
        break;
    }

    assert((*frame).find(value) != (*frame).end());
    size_t offset_result = (*frame)[value].offset;
    gen_sw(reg_result, offset_result, "sp");

    register_counter = register_counter_init;
}

void Visit(const koopa_raw_store_t &store){
    if(store.value->kind.tag == KOOPA_RVT_FUNC_ARG_REF){
        size_t idx = store.value->kind.data.func_arg_ref.index;
        if(idx < 8){
            size_t offset_dest = (*frame)[store.dest].offset;
            rs2 = "a" + std::to_string(idx);
            gen_sw(rs2, (int32_t)offset_dest, "sp");
        }
        else{
            size_t frame_size = map_frame2size[frame];
            idx -= 8;
            size_t offset_src = frame_size + idx * 4;
            rs = "t" + std::to_string(register_counter++);
            gen_lw(rs, offset_src, "sp");

            size_t offset_dest = (*frame)[store.dest].offset;
            gen_sw(rs, offset_dest, "sp");
            register_counter--;
        }
    }
    else{
        int register_counter_original = register_counter;

        if(store.value->kind.tag == KOOPA_RVT_INTEGER){
            rd = "t" + std::to_string(register_counter++);
            gen_li(rd, store.value->kind.data.integer.value);
        }
        else{

// std::cerr << store.value->kind.tag << "," << store.value->name  << "," << store.value->ty->tag << "\n";

            assert((*frame).find(store.value) != (*frame).end());
            size_t offset_src = (*frame)[store.value].offset;
            rs = "t" + std::to_string(register_counter++);
            gen_lw(rs, offset_src, "sp");
        }

        if(store.dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
            rd = "t" + std::to_string(register_counter++);
            gen_la(rd, globl2name[store.dest]);

            rs2 = "t" + std::to_string(register_counter - 2);
            rs1 = "t" + std::to_string(register_counter - 1);
            gen_sw(rs2, 0, rs1);
        }
        else if(store.dest->kind.tag == KOOPA_RVT_GET_ELEM_PTR){
            assert((*frame).find(store.dest) != (*frame).end());
            size_t offset_dest = (*frame)[store.dest].offset;

            rs = "t" + std::to_string(register_counter++);
            gen_lw(rs, (int32_t)offset_dest, "sp");

            rs2 = "t" + std::to_string(register_counter - 2);
            rs1 = "t" + std::to_string(register_counter - 1);
            gen_sw(rs2, 0, rs1);
        }
        else{
            assert((*frame).find(store.dest) != (*frame).end());
            size_t offset_dest = (*frame)[store.dest].offset;

            rs2 = "t" + std::to_string(register_counter - 1);
            gen_sw(rs2, (int32_t)offset_dest, "sp");
        }

        register_counter = register_counter_original;
    }
}

void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value){

std::cerr << "?" <<    load.src->kind.tag << "\n";

    if(load.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
        rd = "t" + std::to_string(register_counter++);
        gen_la(rd, globl2name[load.src]);
        gen_lw(rd, 0, rd);
    }
    else if(load.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR){
        assert((*frame).find(load.src) != (*frame).end());
        size_t offset_src = (*frame)[load.src].offset;
        rs = "t" + std::to_string(register_counter++);
        gen_lw(rs, (int32_t)offset_src, "sp");
        gen_lw(rs, 0, rs);
    }
    else{
        assert((*frame).find(load.src) != (*frame).end());
        size_t offset_src = (*frame)[load.src].offset;
        rs = "t" + std::to_string(register_counter++);
        gen_lw(rs, (int32_t)offset_src, "sp");
    }

    assert((*frame).find(value) != (*frame).end());
    size_t offset_dest = (*frame)[value].offset;
    rs2 = "t" + std::to_string(register_counter - 1);
    gen_sw(rs2, (int32_t)offset_dest, "sp");
    --register_counter;
}

void Visit(const koopa_raw_branch_t &branch){
    if(branch.cond->kind.tag == KOOPA_RVT_INTEGER){
        rd = "t" + std::to_string(register_counter++);
        gen_li(rd, branch.cond->kind.data.integer.value);
        gen_bnez(rd, branch.true_bb->name + 1);
    }
    else{
        assert((*frame).find(branch.cond) != (*frame).end());
        size_t offset_cond = (*frame)[branch.cond].offset;

        rs = "t" + std::to_string(register_counter++);
        gen_lw(rs, (int32_t)offset_cond, "sp");
        gen_bnez(rs, branch.true_bb->name + 1);
    }
    gen_j(branch.false_bb->name + 1);
    --register_counter;
}

void Visit(const koopa_raw_jump_t &jump){
    gen_j(jump.target->name + 1);
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
                rd = "a" + std::to_string(idx);
                gen_li(rd, param->kind.data.integer.value);
            }
            else{
                assert((*frame).find(param) != (*frame).end());
                size_t offset_param = (*frame)[param].offset;
                rs = "a" + std::to_string(idx);
                gen_lw(rs, (int32_t)offset_param, "sp");
            }
        }
        else{
            if(param->kind.tag == KOOPA_RVT_INTEGER){
                rd = "t" + std::to_string(register_counter++);
                gen_li(rd, param->kind.data.integer.value);
                size_t offset_dest = (idx - 8) * 4;
                gen_sw(rd, (int32_t)offset_dest, "sp");
                --register_counter;
            }
            else{
                assert((*frame).find(param) != (*frame).end());
                size_t offset_param = (*frame)[param].offset;
                rs = "t" + std::to_string(register_counter++);
                gen_lw(rs, (int32_t)offset_param, "sp");
                size_t offset_dest = (idx - 8) * 4;
                gen_sw(rs, (int32_t)offset_dest, "sp");
                --register_counter;
            }
        }
    }
    gen_call(call.callee->name + 1);

    if(value->ty->tag != KOOPA_RTT_UNIT){
        assert((*frame).find(value) != (*frame).end());
        size_t offset_dest = (*frame)[value].offset;
        gen_sw("a0", (int32_t)offset_dest, "sp");
    }
}

void Visit(const koopa_raw_global_alloc_t &globl_alloc, const koopa_raw_value_t &value){

std::cerr << "?" << globl_alloc.init->kind.tag << "\t" << globl_alloc.init->kind.data.integer.value << "\n";

    std::cout << "\t.globl\t" << value->name + 1 << std::endl;
    std::cout << value->name + 1 << ":" << std::endl;

    globl2name[value] = std::string(value->name + 1);

    riscv_gen_initializer(globl_alloc.init);
}

void Visit(const koopa_raw_get_elem_ptr_t &get_elem_ptr, const koopa_raw_value_t &value){
    if(get_elem_ptr.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
        rd = "t" + std::to_string(register_counter++);
        gen_la(rd, globl2name[get_elem_ptr.src]);
        // gen_lw(rd, 0, rd);
    }
    else if(get_elem_ptr.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR){
        rs = "t" + std::to_string(register_counter++);
        size_t offset_src = (*frame)[get_elem_ptr.src].offset;
        gen_lw(rs, (int32_t)offset_src, "sp");
    }
    else{
        assert((*frame).find(get_elem_ptr.src) != (*frame).end());
        size_t offset_base = (*frame)[get_elem_ptr.src].offset;
        rd = "t" + std::to_string(register_counter++);
        gen_addi(rd, "sp", (int32_t)offset_base);
    }

    if(get_elem_ptr.index->kind.tag == KOOPA_RVT_INTEGER){
        rd = "t" + std::to_string(register_counter++);
        gen_li(rd, get_elem_ptr.index->kind.data.integer.value);
    }
    else{
        assert((*frame).find(get_elem_ptr.index) != (*frame).end());
        size_t offset_idx = (*frame)[get_elem_ptr.index].offset;
        rs = "t" + std::to_string(register_counter++);
        gen_lw(rs, (int32_t)offset_idx, "sp");
    }

    assert(get_elem_ptr.src->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY);
    size_t elem_size = size_of_type(get_elem_ptr.src->ty->data.pointer.base->data.array.base);
    assert(elem_size > 0);

    if((elem_size & (elem_size - 1)) == 0){
        int shift = 0;
        while(1){
            elem_size = elem_size >> 1;
            if(elem_size == 0){
                break;
            }
            shift += 1;
        }
        rd = "t" + std::to_string(register_counter++);
        gen_li(rd, shift);
        rs1 = "t" + std::to_string(register_counter - 2);
        gen_sll(rs1, rs1, rd);
        --register_counter;
    }
    else{
        rd = "t" + std::to_string(register_counter++);
        gen_li(rd, elem_size);
        rs1 = "t" + std::to_string(register_counter - 2);
        gen_mul(rs1, rs1, rd);
        --register_counter;
    }

    rs1 = "t" + std::to_string(register_counter - 2);
    rs2 = "t" + std::to_string(register_counter - 1);
    gen_add(rs1, rs1, rs2);
    --register_counter;

    assert((*frame).find(value) != (*frame).end());
    size_t offset_dest = (*frame)[value].offset;
    rs2 = "t" + std::to_string(register_counter - 1);
    gen_sw(rs2, (int32_t)offset_dest, "sp");
    --register_counter;
}


/*
    Abandoned for now;
    might be transformed into value_t2reg
    when implement advanced reg alloc
*/
// bool operator<(koopa_raw_binary_t a, koopa_raw_binary_t b){
//     return ((uint32_t)a.op == (uint32_t)b.op) ?
//             (((uint64_t)a.lhs == (uint64_t)b.lhs) ?
//                 ((uint64_t)a.rhs < (uint64_t)b.rhs)
//                 : ((uint64_t)a.lhs < (uint64_t)b.lhs))
//             : ((uint32_t)a.op < (uint32_t)b.op);
// }
// static std::map<koopa_raw_binary_t, std::string> binary2reg;
