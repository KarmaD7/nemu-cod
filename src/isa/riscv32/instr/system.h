#ifndef __ICS_EXPORT

#define csr_difftest()  IFNDEF(CONFIG_DIFFTEST_REF_NEMU, difftest_skip_dut(1, 3))
#define priv_difftest() IFNDEF(CONFIG_DIFFTEST_REF_NEMU, difftest_skip_dut(1, 2))

def_EHelper(csrrw) {
  csr_difftest();
  rtl_hostcall(s, HOSTCALL_CSR, ddest, dsrc1, NULL, id_src2->imm);
  rtl_priv_next(s);
}

def_EHelper(csrrs) {
  csr_difftest();
  rtl_hostcall(s, HOSTCALL_CSR, s0, NULL, NULL, id_src2->imm);
  rtl_or(s, s1, s0, dsrc1);
  rtl_mv(s, ddest, s0);
  rtl_hostcall(s, HOSTCALL_CSR, NULL, s1, NULL, id_src2->imm);
  rtl_priv_next(s);
}

def_EHelper(csrrc) {
  csr_difftest();
  rtl_hostcall(s, HOSTCALL_CSR, s0, NULL, NULL, id_src2->imm);
  rtl_not(s, s1, dsrc1);
  rtl_and(s, s1, s0, s1);
  rtl_mv(s, ddest, s0);
  rtl_hostcall(s, HOSTCALL_CSR, NULL, s1, NULL, id_src2->imm);
  rtl_priv_next(s);
}

def_EHelper(csrrwi) {
  csr_difftest();
  rtl_li(s, s0, s->isa.instr.i.rs1);
  rtl_hostcall(s, HOSTCALL_CSR, ddest, s0, NULL, id_src2->imm);
  rtl_priv_next(s);
}

def_EHelper(csrrsi) {
  csr_difftest();
  rtl_hostcall(s, HOSTCALL_CSR, s0, NULL, NULL, id_src2->imm);
  rtl_li(s, s1, s->isa.instr.i.rs1);
  rtl_or(s, s1, s0, s1);
  rtl_mv(s, ddest, s0);
  rtl_hostcall(s, HOSTCALL_CSR, NULL, s1, NULL, id_src2->imm);
  rtl_priv_next(s);
}

def_EHelper(csrrci) {
  csr_difftest();
  rtl_hostcall(s, HOSTCALL_CSR, s0, NULL, NULL, id_src2->imm);
  rtl_li(s, s1, s->isa.instr.i.rs1);
  rtl_not(s, s1, s1);
  rtl_and(s, s1, s0, s1);
  rtl_mv(s, ddest, s0);
  rtl_hostcall(s, HOSTCALL_CSR, NULL, s1, NULL, id_src2->imm);
  rtl_priv_next(s);
}

def_EHelper(ecall) {
  priv_difftest();
  rtl_trap(s, s->pc, 9);
  rtl_priv_jr(s, t0);
}

def_EHelper(sret) {
  priv_difftest();
  rtl_hostcall(s, HOSTCALL_PRIV, s0, NULL, NULL, 0x102);
  rtl_priv_jr(s, s0);
}

def_EHelper(mret) {
  priv_difftest();
  rtl_hostcall(s, HOSTCALL_PRIV, s0, NULL, NULL, 0x302);
  rtl_priv_jr(s, s0);
}

def_EHelper(sfence_vma) {
  priv_difftest();
  rtl_hostcall(s, HOSTCALL_PRIV, NULL, dsrc1, NULL, 0x120);
  rtl_priv_next(s);
}
#endif
