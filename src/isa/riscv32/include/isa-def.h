#ifndef __ISA_RISCV32_H__
#define __ISA_RISCV32_H__

#include <common.h>
#include "../local-include/csr.h"

struct ExecutionGuide {
  // force raise exception
  bool force_raise_exception;
  uint64_t exception_num;
  uint64_t mtval;
  uint64_t stval;
  // force set jump target
  bool force_set_jump_target;
  uint64_t jump_target;
};

struct DebugInfo {
  uint64_t current_pc;
};

#ifdef CONFIG_QUERY_REF
typedef enum RefQueryType {
  REF_QUERY_MEM_EVENT
} RefQueryType;

struct MemEventQueryResult {
  uint64_t pc;
  bool mem_access;
  bool mem_access_is_load;
  // uint64_t mem_access_paddr;
  uint64_t mem_access_vaddr;
  // uint64_t mem_access_result;
};
#endif

typedef struct {
  struct {
    rtlreg_t _32;
  } gpr[32];

#ifndef __ICS_EXPORT
  uint32_t pc;
  uint32_t mstatus, mcause, mepc;
  uint32_t sstatus, scause, sepc;

  uint32_t satp, mip, mie, mscratch, sscratch, mideleg, medeleg;
  uint32_t mtval, stval, mtvec, stvec;
  uint32_t mode;
  bool INTR;
#endif
} riscv32_CPU_state;

// decode
typedef struct {
  union {
    struct {
      uint32_t opcode1_0 : 2;
      uint32_t opcode6_2 : 5;
      uint32_t rd        : 5;
      uint32_t funct3    : 3;
      uint32_t rs1       : 5;
      int32_t  simm11_0  :12;
    } i;
    struct {
      uint32_t opcode1_0 : 2;
      uint32_t opcode6_2 : 5;
      uint32_t imm4_0    : 5;
      uint32_t funct3    : 3;
      uint32_t rs1       : 5;
      uint32_t rs2       : 5;
      int32_t  simm11_5  : 7;
    } s;
    struct {
      uint32_t opcode1_0 : 2;
      uint32_t opcode6_2 : 5;
      uint32_t rd        : 5;
      uint32_t imm31_12  :20;
    } u;
#ifndef __ICS_EXPORT
    struct {
      uint32_t opcode1_0 : 2;
      uint32_t opcode6_2 : 5;
      uint32_t rd        : 5;
      uint32_t funct3    : 3;
      uint32_t rs1       : 5;
      uint32_t rs2       : 5;
      uint32_t funct7    : 7;
    } r;
    struct {
      uint32_t opcode1_0 : 2;
      uint32_t opcode6_2 : 5;
      uint32_t imm11     : 1;
      uint32_t imm4_1    : 4;
      uint32_t funct3    : 3;
      uint32_t rs1       : 5;
      uint32_t rs2       : 5;
      uint32_t imm10_5   : 6;
      int32_t  simm12    : 1;
    } b;
    struct {
      uint32_t opcode1_0 : 2;
      uint32_t opcode6_2 : 5;
      uint32_t rd        : 5;
      uint32_t imm19_12  : 8;
      uint32_t imm11     : 1;
      uint32_t imm10_1   :10;
      int32_t  simm20    : 1;
    } j;
    struct {
      uint32_t pad7      :20;
      uint32_t csr       :12;
    } csr;
#endif
    uint32_t val;
  } instr;
} riscv32_ISADecodeInfo;

enum { MODE_U = 0, MODE_S, MODE_H, MODE_M };
#ifdef __ICS_EXPORT
#define isa_mmu_state() (MMU_DIRECT)
#else
#define isa_mmu_state() (satp->mode ? MMU_TRANSLATE : MMU_DIRECT)
#endif
#define isa_mmu_check(vaddr, len, type) isa_mmu_state()

#endif
