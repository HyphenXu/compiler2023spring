#ifndef FRAME_H
#define FRAME_H

#include <map>
#include <string>

#include "koopa.h"

#define STACK_ALIGNMENT 16
#define SIZE_INT32      4
#define STACK_IMM_NEG_MAX   2048
#define STACK_IMM_POS_MAX   2047

typedef struct{
    size_t offset;
    size_t array_elem_size;
} frame_entry_t;

// typedef std::map<std::string, frame_entry_t> frame_t;
typedef std::map<koopa_raw_value_t, frame_entry_t> frame_t;
typedef std::map<std::string, frame_t> frames_t;
typedef std::map<frame_t *, size_t> map_frame2size_t;
typedef std::map<frame_t *, bool> map_frame2bool_t;

extern frames_t frames;
extern frame_t *frame;
extern map_frame2size_t map_frame2size;
extern map_frame2bool_t map_frame2is_with_call;

size_t size_of_type(const koopa_raw_type_t &ty);

void func_alloc_frame(const koopa_raw_function_t &func);

#endif /**< src/frame.h */
