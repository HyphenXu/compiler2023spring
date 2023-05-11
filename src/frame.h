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

frames_t frames;
frame_t *frame;
map_frame2size_t map_frame2size;

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

    frames[func->name] = frame_t();
    frame = &(frames[func->name]);

    frame_size = 0;
    assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
    for(size_t i = 0; i < func->bbs.len; ++i){
        auto bb = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
        assert(bb->insts.kind == KOOPA_RSIK_VALUE);
        for(size_t j = 0; j < bb->insts.len; ++j){
            auto ptr = reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[j]);
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

    frame_size = (frame_size + STACK_ALIGNMENT - 1 )
                / STACK_ALIGNMENT
                * STACK_ALIGNMENT;

    map_frame2size[frame] = frame_size;
}

#endif /**< src/frame.h */
