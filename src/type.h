#ifndef TYPE_H
#define TYPE_H

typedef enum{
    COMP_UNIT_FUNC_DEF,
    COMP_UNIT_DECL,
} comp_unit_type_t;

typedef enum{
    DECL_CONST_DECL,
    DECL_VAR_DECL,
} decl_type_t;

typedef enum{
    BLOCK_ITEM_DECL,
    BLOCK_ITEM_GENERAL_STMT,
} block_item_type_t;

typedef enum{
    GENERAL_STMT_STMT,
    GENERAL_STMT_OPEN_STMT,
} general_stmt_type_t;

typedef enum{
    STMT_ASSIGN,
    STMT_EXP,
    STMT_BLOCK,
    STMT_RETURN,
    STMT_IF_STMT,
    STMT_WHILE_STMT,
    STMT_BREAK,
    STMT_CONTINUE,
} stmt_type_t;

typedef enum{
    OPEN_STMT_IF_GENERAL_STMT,
    OPEN_STMT_IF_STMT_OPEN_STMT,
    OPEN_STMT_WHILE_OPEN_STMT,
} open_stmt_type_t;

typedef enum{
    PRIMARY_EXP_EXP,
    PRIMARY_EXP_L_VAL,
    PRIMARY_EXP_NUMBER,
} primary_exp_type_t;

typedef enum{
    UNARY_EXP_PRIMARY_EXP,
    UNARY_EXP_UNARY_OP_EXP,
    UNARY_EXP_FUNCTION_CALL,
} unary_exp_type_t;

typedef enum{
    MUL_EXP_UNARY,
    MUL_EXP_MUL,
    MUL_EXP_DIV,
    MUL_EXP_MOD,
} mul_exp_type_t;

typedef enum{
    ADD_EXP_MUL,
    ADD_EXP_ADD,
    ADD_EXP_SUB,
} add_exp_type_t;

typedef enum{
    REL_EXP_ADD,
    REL_EXP_LT,
    REL_EXP_GT,
    REL_EXP_ORDERED,
} rel_exp_type_t;

typedef enum{
    EQ_EXP_REL,
    EQ_EXP_UNORDERED,
} eq_exp_type_t;

typedef enum{
    L_AND_EXP_EQ,
    L_AND_EXP_L_AND,
} l_and_exp_type_t;

typedef enum{
    L_OR_EXP_L_AND,
    L_OR_EXP_L_OR,
} l_or_exp_type_t;

#endif /**< src/type.h */
