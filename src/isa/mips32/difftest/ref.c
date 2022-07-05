#include <isa.h>
#include <difftest.h>
#include "../local-include/intr.h"

void isa_difftest_regcpy(void *dut, bool direction) {
  if (direction == DIFFTEST_TO_REF) memcpy(&cpu, dut, DIFFTEST_REG_SIZE);
  else memcpy(dut, &cpu, DIFFTEST_REG_SIZE);
}

void isa_difftest_raise_intr(word_t NO) {
  cpu.pc = raise_intr(NO, cpu.pc);
}

void isa_difftest_csrcpy(void *dut, bool direction) {
  // TODO for mips
  // if (direction == DIFFTEST_TO_REF) {
  //   memcpy(csr_array, dut, 4096 * sizeof(rtlreg_t));
  // } else {
  //   memcpy(dut, csr_array, 4096 * sizeof(rtlreg_t));
  // }
}

void isa_difftest_uarchstatus_cpy(void *dut, bool direction) {
  // May TODO? it's for lr & sc in riscv
  // if (direction == DIFFTEST_TO_REF) {
  //   struct SyncState* ms = (struct SyncState*)dut;
  //   cpu.lr_valid = ms->lrscValid;
  // } else {
  //   struct SyncState ms;
  //   ms.lrscValid = cpu.lr_valid;
  //   ms.lrscAddr = cpu.lr_addr;
  //   memcpy(dut, &ms, sizeof(struct SyncState));
  // }
}
