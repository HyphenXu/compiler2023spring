#include "riscv.h"

#include <iostream>
#include <cassert>

static int temp_label_id = 0;

void gen_add(const std::string &rd, const std::string &rs1,
             const std::string &rs2){
    std::cout << "\tadd\t" << rd << ", " << rs1 << ", " << rs2;
    std::cout << std::endl;
}

void gen_sub(const std::string &rd, const std::string &rs1,
             const std::string &rs2){
    std::cout << "\tsub\t" << rd << ", " << rs1 << ", " << rs2;
    std::cout << std::endl;
}

void gen_li(const std::string &rd, int32_t imm){
    std::cout << "\tli\t" << rd << ", " << imm;
    std::cout << std::endl;
}

void gen_addi(const std::string &rd, const std::string &rs1, int32_t imm){
    if(imm > IMM12_MAX || imm < IMM12_MIN){
        if(rd == rs1){
            /* TODO: careful with this*/
            std::string rtemp = "t" + std::to_string(register_counter++);
            gen_li(rtemp, imm);
            gen_add(rd, rs1, rtemp);
            --register_counter;
        }
        else{
            gen_li(rd, imm);
            gen_add(rd, rs1, rd);
        }
    }
    else{
        std::cout << "\taddi\t" << rd << ", " << rs1 << ", " << imm;
        std::cout << std::endl;
    }
}

void gen_sw(const std::string &rs2, int32_t imm, const std::string &rs1){
    if(imm > IMM12_MAX || imm < IMM12_MIN){
        /* TODO: careful with this*/
        std::string rtemp = "t" + std::to_string(register_counter++);
        gen_li(rtemp, imm);
        gen_add(rtemp, rtemp, rs1);
        gen_sw(rs2, 0, rtemp);
        --register_counter;
    }
    else{
        std::cout << "\tsw\t" << rs2 << ", " << imm << "(" << rs1 << ")";
        std::cout << std::endl;
    }
}

void gen_lw(const std::string &rs, int32_t imm, const std::string &rd){
    if(imm > IMM12_MAX || imm < IMM12_MIN){
        if(rs == rd){
            /* TODO: careful with this*/
            std::string rtemp = "t" + std::to_string(register_counter++);
            gen_li(rtemp, imm);
            gen_add(rtemp, rtemp, rd);
            gen_lw(rs, 0, rtemp);
            --register_counter;
        }
        else{
            gen_li(rs, imm);
            gen_add(rs, rs, rd);
            gen_lw(rs, 0, rs);
        }
    }
    else{
        std::cout << "\tlw\t" << rs << ", " << imm << "(" << rd << ")";
        std::cout << std::endl;
    }
}

void gen_ret(){
    std::cout << "\tret" << std::endl;
}

void gen_la(const std::string &rd, const std::string &label){
    std::cout << "\tla\t" << rd << ", " << label;
    std::cout << std::endl;
}

void gen_bnez(const std::string &rs, const std::string &label){
    // std::cout << "\tbnez\t" << rs << ", " << label;
    // std::cout << std::endl;
    std::string temp_label = "temp_label_" + std::to_string(temp_label_id++);
    std::cout << "\tbnez\t" << rs << ", " << temp_label;
    std::cout << std::endl;
    std::cout << "\tj\t" << "after_" << temp_label << std::endl;
    std::cout << temp_label << ":" << std::endl;
    gen_j(label);
    std::cout << "after_" << temp_label << ":" << std::endl;
}

void gen_j(const std::string &label){
    std::cout << "\tj\t" << label;
    std::cout << std::endl;
    // /* TODO: careful with this*/
    // std::string rtemp = "t" + std::to_string(register_counter++);
    // gen_la(rtemp, label);
    // std::cout << "\tjr\t" << rtemp;
    // std::cout << std::endl;
    // --register_counter;
}

void gen_call(const std::string &label){
    std::cout << "\tcall\t" << label;
    std::cout << std::endl;
}

void gen_sll(const std::string &rd, const std::string &rs1,
             const std::string &rs2){
    std::cout << "\tsll\t" << rd << ", " << rs1 << ", " << rs2;
    std::cout << std::endl;
}

void gen_mul(const std::string &rd, const std::string &rs1,
             const std::string &rs2){
    std::cout << "\tmul\t" << rd << ", " << rs1 << ", " << rs2;
    std::cout << std::endl;
}