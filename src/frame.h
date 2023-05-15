#ifndef FRAME_H
#define FRAME_H

#include <iostream>
#include <map>
#include <string>
#include <cassert>

#include "koopa.h"

#define STACK_ALIGNMENT 16
#define SIZE_INT32      4
#define STACK_IMM_NEG_MAX   2048
#define STACK_IMM_POS_MAX   2047

typedef struct{
    int offset;
} frame_entry_t;

// typedef std::map<std::string, frame_entry_t> frame_t;
typedef std::map<koopa_raw_value_t, frame_entry_t> frame_t;
typedef std::map<std::string, frame_t> frames_t;
typedef std::map<frame_t *, size_t> map_frame2size_t;
typedef std::map<frame_t *, bool> map_frame2bool_t;

frames_t frames;
frame_t *frame;
map_frame2size_t map_frame2size;
map_frame2bool_t map_frame2is_with_call;

int size_of_type(const koopa_raw_type_t &ty){
    switch (ty->tag)
    {
    case KOOPA_RTT_INT32:
        return SIZE_INT32;
        break;
    case KOOPA_RTT_POINTER:
        /* TODO: might be wrong? */
        return size_of_type(ty->data.pointer.base);
        break;
    case KOOPA_RTT_FUNCTION:
        /* TODO */
        assert(false);
        break;
    case KOOPA_RTT_ARRAY:
        /* TODO */
        assert(false);
        break;
    default:
        assert(false);
        break;
    }
    return 0;
}

/* func_scan_inst_for_stack_space */
void func_alloc_frame(const koopa_raw_function_t &func){
    size_t frame_size;
    bool is_with_call;
    size_t max_num_args;

    frames[func->name] = frame_t();
    frame = &(frames[func->name]);

    frame_size = 0;
    max_num_args = 0;
    is_with_call = false;
    assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);

    for(size_t i = 0; i < func->bbs.len; ++i){
        auto bb = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
        assert(bb->insts.kind == KOOPA_RSIK_VALUE);
        for(size_t j = 0; j < bb->insts.len; ++j){
            auto ptr = reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[j]);
            std::cerr << "\tvalue ret type: " << ptr->ty->tag << "\t";
            std::cerr << "\tvalue inst kind: " << ptr->kind.tag << "\n";
            if(ptr->kind.tag == KOOPA_RVT_CALL){
                is_with_call = true;
                size_t len_call_args = ptr->kind.data.call.args.len;
                std::cerr << "\t\tcall: num of args " << ptr->kind.data.call.args.len << "\n";
                if(len_call_args > max_num_args){
                    max_num_args = len_call_args;
                }
            }
        }
    }

    frame_size += (max_num_args > 8) ? (4 * (max_num_args - 8)) : 0;

    for(size_t i = 0; i < func->bbs.len; ++i){
        auto bb = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
        assert(bb->insts.kind == KOOPA_RSIK_VALUE);
        for(size_t j = 0; j < bb->insts.len; ++j){
            auto ptr = reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[j]);
            std::cerr << "\tvalue ret type: " << ptr->ty->tag << "\n";
            switch (ptr->ty->tag)
            {
            case KOOPA_RTT_INT32:
                /* Intermediate result */
                assert(ptr->name == nullptr);
                (*frame)[ptr].offset = frame_size;
                frame_size += size_of_type(ptr->ty);
                break;
            case KOOPA_RTT_UNIT:
                /* No need of alloc */
                break;
            case KOOPA_RTT_POINTER:
                (*frame)[ptr].offset = frame_size;
                frame_size += size_of_type(ptr->ty);
                /* Alloc instruction */
                break;
            case KOOPA_RTT_ARRAY:
                /* TODO */
                assert(false);
                break;
            case KOOPA_RTT_FUNCTION:
                /* TODO */
                assert(false);
                break;
            default:
                assert(false);
                break;
            }
        }
    }

    if(is_with_call){
        frame_size += 4;
    }

    frame_size = (frame_size + STACK_ALIGNMENT - 1 )
                / STACK_ALIGNMENT
                * STACK_ALIGNMENT;

    map_frame2size[frame] = frame_size;
    map_frame2is_with_call[frame] = is_with_call;
}

#endif /**< src/frame.h */
