#ifndef RISCV_H
#define RISCV_H

#include <string>

#define IMM12_MAX   2047
#define IMM12_MIN   (-IMM12_MAX - 1)
#define IMM32_MAX   2147483647
#define IMM32_MIN   (-IMM32_MAX - 1)

/* TODO: careful with this*/
extern int register_counter;

void gen_add(const std::string &rd, const std::string &rs1,
             const std::string &rs2);
void gen_sub(const std::string &rd, const std::string &rs1,
             const std::string &rs2);
void gen_sll(const std::string &rd, const std::string &rs1,
             const std::string &rs2);
void gen_mul(const std::string &rd, const std::string &rs1,
             const std::string &rs2);
void gen_li(const std::string &rd, int32_t imm);
void gen_addi(const std::string &rd, const std::string &rs1, int32_t imm);
void gen_sw(const std::string &rs2, int32_t imm, const std::string &rs1);
void gen_lw(const std::string &rs, int32_t imm, const std::string &rd);
void gen_ret();
void gen_la(const std::string &rd, const std::string &label);
void gen_bnez(const std::string &rs, const std::string &label);
void gen_j(const std::string &label);
void gen_call(const std::string &label);

#endif /**< src/riscv.h */
