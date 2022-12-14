#include <isa.h>
#include <memory/paddr.h>
#include "../local-include/csr.h"

// this is not consistent with uint8_t
// but it is ok since we do not access the array directly
static const uint32_t img [] = {
  0x800002b7,  // lui t0,0x80000
  0x0002a023,  // sw  zero,0(t0)
  0x0002a503,  // lw  a0,0(t0)
  0x0000006b,  // nemu_trap
};

void init_csr();
#ifndef CONFIG_SHARE
void init_clint();
#endif
void init_device();


static void restart() {
  init_csr();
  /* Set the initial program counter. */
  cpu.pc = RESET_VECTOR;

  /* The zero register is always 0. */
  cpu.mode = MODE_M;
  cpu.gpr[0]._32 = 0;
#ifndef __ICS_EXPORT
  // sstatus->val = 0x000c0100;
  // mstatus->val = 0x000c0100;
  sstatus->val = 0;
  mstatus->val = 0;
  pmpcfg0->val = 0;
  pmpcfg1->val = 0;
  pmpcfg2->val = 0;
  pmpcfg3->val = 0;
#endif

  IFNDEF(CONFIG_SHARE, init_clint());
  IFDEF(CONFIG_SHARE, init_device());
}

void init_isa() {
  /* Load built-in image. */
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));

  /* Initialize this virtual computer system. */
  restart();
}
