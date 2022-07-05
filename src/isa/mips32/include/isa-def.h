#ifndef __ISA_MIPS32_H__
#define __ISA_MIPS32_H__

#include <common.h>

// Execution Guide generated by DUT
struct ExecutionGuide {
  // force raise exception
  bool force_raise_exception;
  uint32_t exception_num;
  // uint32_t mtval;
  // uint32_t stval;
  // force set jump target
  bool force_set_jump_target;
  uint32_t jump_target;
};

struct DebugInfo {
  uint32_t current_pc;
};

#ifdef CONFIG_QUERY_REF
typedef enum RefQueryType {
  REF_QUERY_MEM_EVENT
} RefQueryType;

struct MemEventQueryResult {
  uint32_t pc;
  bool mem_access;
  bool mem_access_is_load;
  // uint64_t mem_access_paddr;
  uint64_t mem_access_vaddr;
  // uint64_t mem_access_result;
};
#endif

// reg
typedef struct {
  struct {
    uint32_t _32;
  } gpr[32];

#ifdef __ICS_EXPORT
  rtlreg_t pad[5];

  vaddr_t pc;
#else
  union {
    struct {
      uint32_t ie:  1;
      uint32_t exl: 1;
      uint32_t erl: 1;
      uint32_t ksu: 2;
      uint32_t pad1: 3;
      uint32_t im: 8;
      uint32_t pad2: 3;
      uint32_t nmi: 1;
      uint32_t sr: 1;
      uint32_t ts: 1;
      uint32_t bev: 1;
      uint32_t dontcare: 5;
      uint32_t cu: 4;
    };
    uint32_t val;
  } status;

  rtlreg_t lo, hi;
  uint32_t badvaddr;
  union {
    struct {
      uint32_t pad;
      /* data */
    };
    uint32_t val;
  } cause;
  
  vaddr_t pc;
  uint32_t epc;

  union {
    struct {
      uint32_t ASID: 8;
      uint32_t pad : 5;
      uint32_t VPN2:19;
    };
    uint32_t val;
  } entryhi;
  uint32_t entrylo0, entrylo1;
  uint32_t index;

  bool INTR;

  bool guided_exec;
  struct ExecutionGuide execution_guide;

  struct DebugInfo debug;
#endif
} mips32_CPU_state;

// decode
typedef struct {
  union {
    struct {
      int32_t  simm   : 16;
      uint32_t rt     :  5;
      uint32_t rs     :  5;
      uint32_t opcode :  6;
    } i;
    struct {
      uint32_t imm    : 16;
      uint32_t rt     :  5;
      uint32_t rs     :  5;
      uint32_t opcode :  6;
    } iu;
#ifndef __ICS_EXPORT
    struct {
      uint32_t target : 26;
      uint32_t opcode :  6;
    } j;
#endif
    struct {
      uint32_t func   : 6;
      uint32_t sa     : 5;
      uint32_t rd     : 5;
      uint32_t rt     : 5;
      uint32_t rs     : 5;
      uint32_t opcode : 6;
    } r;
    uint32_t val;
  } instr;
} mips32_ISADecodeInfo;

#define isa_mmu_state() (MMU_DYNAMIC)
#ifdef __ICS_EXPORT
#define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)
#else
#define isa_mmu_check(vaddr, len, type) ((vaddr & 0x80000000u) == 0 ? MMU_TRANSLATE : MMU_DIRECT)
#endif

enum { MODE_U = 0, MODE_S };

#endif
