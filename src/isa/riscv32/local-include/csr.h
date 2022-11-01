#ifndef __CSR_H__
#define __CSR_H__

#include <common.h>

// SHARE mode does not support mtime
#ifndef CONFIG_SHARE
#define CSRS(f) \
  f(mstatus    , 0x300) f(misa       , 0x301) f(medeleg    , 0x302) f(mideleg    , 0x303) \
  f(mie        , 0x304) f(mtvec      , 0x305) f(mcounteren , 0x306) \
  f(mscratch   , 0x340) f(mepc       , 0x341) f(mcause     , 0x342) \
  f(mtval      , 0x343) f(mip        , 0x344) \
  f(pmpcfg0    , 0x3a0) f(pmpcfg1    , 0x3a1) f(pmpcfg2    , 0x3a2) f(pmpcfg3    , 0x3a3) \
  f(pmpaddr0   , 0x3b0) f(pmpaddr1   , 0x3b1) f(pmpaddr2   , 0x3b2) f(pmpaddr3   , 0x3b3) \
  f(pmpaddr4   , 0x3b4) f(pmpaddr5   , 0x3b5) f(pmpaddr6   , 0x3b6) f(pmpaddr7   , 0x3b7) \
  f(pmpaddr8   , 0x3b8) f(pmpaddr9   , 0x3b9) f(pmpaddr10  , 0x3ba) f(pmpaddr11  , 0x3bb) \
  f(pmpaddr12  , 0x3bc) f(pmpaddr13  , 0x3bd) f(pmpaddr14  , 0x3be) f(pmpaddr15  , 0x3bf) \
  f(mhartid    , 0xf14) \
  f(sstatus    , 0x100) \
  f(sie        , 0x104) f(stvec      , 0x105) f(scounteren , 0x106) \
  f(sscratch   , 0x140) f(sepc       , 0x141) f(scause     , 0x142) \
  f(stval      , 0x143) f(sip        , 0x144) \
  f(satp       , 0x180) \
  f(mtime      , 0xc01)
#else
#define CSRS(f) \
  f(mstatus    , 0x300) f(misa       , 0x301) f(medeleg    , 0x302) f(mideleg    , 0x303) \
  f(mie        , 0x304) f(mtvec      , 0x305) f(mcounteren , 0x306) \
  f(mscratch   , 0x340) f(mepc       , 0x341) f(mcause     , 0x342) \
  f(mtval      , 0x343) f(mip        , 0x344) \
  f(pmpcfg0    , 0x3a0) f(pmpcfg1    , 0x3a1) f(pmpcfg2    , 0x3a2) f(pmpcfg3    , 0x3a3) \
  f(pmpaddr0   , 0x3b0) f(pmpaddr1   , 0x3b1) f(pmpaddr2   , 0x3b2) f(pmpaddr3   , 0x3b3) \
  f(pmpaddr4   , 0x3b4) f(pmpaddr5   , 0x3b5) f(pmpaddr6   , 0x3b6) f(pmpaddr7   , 0x3b7) \
  f(pmpaddr8   , 0x3b8) f(pmpaddr9   , 0x3b9) f(pmpaddr10  , 0x3ba) f(pmpaddr11  , 0x3bb) \
  f(pmpaddr12  , 0x3bc) f(pmpaddr13  , 0x3bd) f(pmpaddr14  , 0x3be) f(pmpaddr15  , 0x3bf) \
  f(mhartid    , 0xf14) \
  f(sstatus    , 0x100) \
  f(sie        , 0x104) f(stvec      , 0x105) f(scounteren , 0x106) \
  f(sscratch   , 0x140) f(sepc       , 0x141) f(scause     , 0x142) \
  f(stval      , 0x143) f(sip        , 0x144) \
  f(satp       , 0x180)
#endif

#define CSR_STRUCT_START(name) \
  typedef union { \
    struct {

#define CSR_STRUCT_END(name) \
    }; \
    word_t val; \
  } concat(name, _t);

CSR_STRUCT_START(mstatus)
  uint32_t uie : 1;
  uint32_t sie : 1;
  uint32_t pad0: 1;
  uint32_t mie : 1;
  uint32_t upie: 1;
  uint32_t spie: 1;
  uint32_t pad1: 1;
  uint32_t mpie: 1;
  uint32_t spp : 1;
  uint32_t vs:   2;
  uint32_t mpp : 2;
  uint32_t fs  : 2;
  uint32_t xs  : 2;
  uint32_t mprv: 1;
  uint32_t sum : 1;
  uint32_t mxr : 1;
  uint32_t tvm : 1;
  uint32_t tw  : 1;
  uint32_t tsr : 1;
  uint32_t pad3: 8;
  uint32_t sd  : 1;
CSR_STRUCT_END(mstatus)

CSR_STRUCT_START(misa)
  uint32_t extensions: 26;
  uint32_t pad       :  4;
  uint32_t mxl       :  2;
CSR_STRUCT_END(misa)

CSR_STRUCT_START(mtvec)
CSR_STRUCT_END(mtvec)

CSR_STRUCT_START(mcounteren)
CSR_STRUCT_END(mcounteren)

CSR_STRUCT_START(mcause)
  uint32_t code:31;
  uint32_t intr: 1;
CSR_STRUCT_END(mcause)

CSR_STRUCT_START(mepc)
CSR_STRUCT_END(mepc)

CSR_STRUCT_START(medeleg)
CSR_STRUCT_END(medeleg)

CSR_STRUCT_START(mideleg)
CSR_STRUCT_END(mideleg)

CSR_STRUCT_START(mhartid)
CSR_STRUCT_END(mhartid)

CSR_STRUCT_START(mscratch)
CSR_STRUCT_END(mscratch)

CSR_STRUCT_START(mtval)
CSR_STRUCT_END(mtval)

CSR_STRUCT_START(mie)
  uint32_t usie : 1;
  uint32_t ssie : 1;
  uint32_t hsie : 1;
  uint32_t msie : 1;
  uint32_t utie : 1;
  uint32_t stie : 1;
  uint32_t htie : 1;
  uint32_t mtie : 1;
  uint32_t ueie : 1;
  uint32_t seie : 1;
  uint32_t heie : 1;
  uint32_t meie : 1;
CSR_STRUCT_END(mie)

CSR_STRUCT_START(mip)
  uint32_t usip : 1;
  uint32_t ssip : 1;
  uint32_t hsip : 1;
  uint32_t msip : 1;
  uint32_t utip : 1;
  uint32_t stip : 1;
  uint32_t htip : 1;
  uint32_t mtip : 1;
  uint32_t ueip : 1;
  uint32_t seip : 1;
  uint32_t heip : 1;
  uint32_t meip : 1;
CSR_STRUCT_END(mip)

/** pmp */

#define PMP_R     0x01
#define PMP_W     0x02
#define PMP_X     0x04
#define PMP_A     0x18
#define PMP_L     0x80
#define PMP_SHIFT 2
#define PMP_PLATFORMGARIN 12 // log2(4KB)

#define PMP_TOR   0x08
#define PMP_NA4   0x10
#define PMP_NAPOT 0x18

#define CSR_PMPCFG0 0x3a0
#define CSR_PMPCFG1 0x3a1
#define CSR_PMPCFG2 0x3a2
#define CSR_PMPCFG3 0x3a3
#define CSR_PMPADDR0 0x3b0
#define CSR_PMPADDR1 0x3b1
#define CSR_PMPADDR2 0x3b2
#define CSR_PMPADDR3 0x3b3
#define CSR_PMPADDR4 0x3b4
#define CSR_PMPADDR5 0x3b5
#define CSR_PMPADDR6 0x3b6
#define CSR_PMPADDR7 0x3b7
#define CSR_PMPADDR8 0x3b8
#define CSR_PMPADDR9 0x3b9
#define CSR_PMPADDR10 0x3ba
#define CSR_PMPADDR11 0x3bb
#define CSR_PMPADDR12 0x3bc
#define CSR_PMPADDR13 0x3bd
#define CSR_PMPADDR14 0x3be
#define CSR_PMPADDR15 0x3bf
#define NUM_PMP 16 // can change in 0/16/64, but need change the above CSR declare

CSR_STRUCT_START(pmpcfg0)
CSR_STRUCT_END(pmpcfg0)

CSR_STRUCT_START(pmpcfg1)
CSR_STRUCT_END(pmpcfg1)

CSR_STRUCT_START(pmpcfg2)
CSR_STRUCT_END(pmpcfg2)

CSR_STRUCT_START(pmpcfg3)
CSR_STRUCT_END(pmpcfg3)

CSR_STRUCT_START(pmpaddr0)
CSR_STRUCT_END(pmpaddr0)

CSR_STRUCT_START(pmpaddr1)
CSR_STRUCT_END(pmpaddr1)

CSR_STRUCT_START(pmpaddr2)
CSR_STRUCT_END(pmpaddr2)

CSR_STRUCT_START(pmpaddr3)
CSR_STRUCT_END(pmpaddr3)

CSR_STRUCT_START(pmpaddr4)
CSR_STRUCT_END(pmpaddr4)

CSR_STRUCT_START(pmpaddr5)
CSR_STRUCT_END(pmpaddr5)

CSR_STRUCT_START(pmpaddr6)
CSR_STRUCT_END(pmpaddr6)

CSR_STRUCT_START(pmpaddr7)
CSR_STRUCT_END(pmpaddr7)

CSR_STRUCT_START(pmpaddr8)
CSR_STRUCT_END(pmpaddr8)

CSR_STRUCT_START(pmpaddr9)
CSR_STRUCT_END(pmpaddr9)

CSR_STRUCT_START(pmpaddr10)
CSR_STRUCT_END(pmpaddr10)

CSR_STRUCT_START(pmpaddr11)
CSR_STRUCT_END(pmpaddr11)

CSR_STRUCT_START(pmpaddr12)
CSR_STRUCT_END(pmpaddr12)

CSR_STRUCT_START(pmpaddr13)
CSR_STRUCT_END(pmpaddr13)

CSR_STRUCT_START(pmpaddr14)
CSR_STRUCT_END(pmpaddr14)

CSR_STRUCT_START(pmpaddr15)
CSR_STRUCT_END(pmpaddr15)

CSR_STRUCT_START(sstatus)
  uint32_t uie : 1;
  uint32_t sie : 1;
  uint32_t pad0: 2;
  uint32_t upie: 1;
  uint32_t spie: 1;
  uint32_t pad1: 2;
  uint32_t spp : 1;
  uint32_t pad2: 4;
CSR_STRUCT_END(sstatus)

CSR_STRUCT_START(stvec)
CSR_STRUCT_END(stvec)

CSR_STRUCT_START(scounteren)
CSR_STRUCT_END(scounteren)

CSR_STRUCT_START(sie)
  uint32_t usie : 1;
  uint32_t ssie : 1;
  uint32_t pad0 : 2;
  uint32_t utie : 1;
  uint32_t stie : 1;
  uint32_t pad1 : 2;
  uint32_t ueie : 1;
  uint32_t seie : 1;
  uint32_t pad2 : 2;
CSR_STRUCT_END(sie)

CSR_STRUCT_START(sip)
  uint32_t usip : 1;
  uint32_t ssip : 1;
  uint32_t pad0 : 2;
  uint32_t utip : 1;
  uint32_t stip : 1;
  uint32_t pad1 : 2;
  uint32_t ueip : 1;
  uint32_t seip : 1;
  uint32_t pad2 : 2;
CSR_STRUCT_END(sip)

#define SATP_ASID_LEN 9 // max is 16
#define SATP_PADDR_LEN (PADDRBITS-12) // max is 44
#define SATP_ASID_MAX_LEN 9
#define SATP_PADDR_MAX_LEN 22

#define SATP_MODE_MASK (1U << (SATP_ASID_MAX_LEN + SATP_PADDR_MAX_LEN))
#define SATP_ASID_MASK (((1U << SATP_ASID_LEN)-1) << SATP_PADDR_MAX_LEN)
#define SATP_PADDR_MASK ((1U << SATP_PADDR_LEN)-1)

#define SATP_MASK (SATP_MODE_MASK | SATP_ASID_MASK | SATP_PADDR_MASK)
#define MASKED_SATP(x) (SATP_MASK & x)

CSR_STRUCT_START(satp)
  uint32_t ppn :22;
  uint32_t asid: 9;
  uint32_t mode: 1;
CSR_STRUCT_END(satp)

CSR_STRUCT_START(scause)
  uint32_t code:31;
  uint32_t intr: 1;
CSR_STRUCT_END(scause)

CSR_STRUCT_START(sepc)
CSR_STRUCT_END(sepc)

CSR_STRUCT_START(stval)
CSR_STRUCT_END(stval)

CSR_STRUCT_START(sscratch)
CSR_STRUCT_END(sscratch)

CSR_STRUCT_START(mtime)
CSR_STRUCT_END(mtime)

#define CSRS_DECL(name, addr) extern concat(name, _t)* const name;
MAP(CSRS, CSRS_DECL)

word_t csrid_read(uint32_t csrid);

// PMP
uint8_t pmpcfg_from_index(int idx);
word_t pmpaddr_from_index(int idx);
word_t pmpaddr_from_csrid(int id);
word_t pmp_tor_mask();

#endif
