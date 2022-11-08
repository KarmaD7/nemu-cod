#include <isa.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>
#include <cpu/cpu.h>
#include "../local-include/csr.h"
#include "../local-include/intr.h"

#ifndef __ICS_EXPORT
/* the 32bit Page Table Entry(second level page table) data structure */
typedef union PageTableEntry {
  struct {
    uint32_t valid  : 1;
    uint32_t read   : 1;
    uint32_t write  : 1;
    uint32_t exec   : 1;
    uint32_t user   : 1;
    uint32_t global : 1;
    uint32_t access : 1;
    uint32_t dirty  : 1;
    uint32_t rsw    : 2;
    uint32_t ppn    :22;
  };
  uint32_t val;
} PTE;

typedef union {
  struct {
    uint32_t pf_off		:12;
    uint32_t pt_idx		:10;
    uint32_t pdir_idx	:10;
  };
  uint32_t addr;
} PageAddr;

typedef struct {
  uint32_t vpn;
  uint32_t ppage;
} TLBEntry;

static TLBEntry TLB[16] = {};
static bool TLBValid[16] = {};

#define PGSHFT 12
#define PGMASK ((1ull << PGSHFT) - 1)
#define PGBASE(pn) (pn << PGSHFT)

// Sv39 page walk
#define PTW_LEVEL 2
#define PTE_SIZE 4
#define VPNMASK 0x3ff
static inline uintptr_t VPNiSHFT(int i) {
  return (PGSHFT) + 10 * i;
}
static inline uintptr_t VPNi(vaddr_t va, int i) {
  return (va >> VPNiSHFT(i)) & VPNMASK;
}


// static int ifetch_mmu_state = MMU_DIRECT;
// static int data_mmu_state = MMU_DIRECT;

// int get_data_mmu_state() {
//   return (data_mmu_state == MMU_DIRECT ? MMU_DIRECT : MMU_TRANSLATE);
// }

// static inline int update_mmu_state_internal(bool ifetch) {
//   uint32_t mode = (mstatus->mprv && (!ifetch) ? mstatus->mpp : cpu.mode);
//   if (mode < MODE_M) {
//     assert(satp->mode == 0 || satp->mode == 8);
//     if (satp->mode == 8) return MMU_TRANSLATE;
//   }
//   return MMU_DIRECT;
// }
#ifdef CONFIG_SHARE
void isa_misalign_data_addr_check(vaddr_t vaddr, int len, int type) {
  if (unlikely((vaddr & (len - 1)) != 0)) {
    int ex =type == MEM_TYPE_WRITE ? EX_SAM : EX_LAM;
    INTR_TVAL_REG(ex) = vaddr;
    longjmp_exception(ex);
  }
}
#endif

static inline int TLB_hash(uint32_t vpn) {
  return vpn % ARRLEN(TLB);
}

static inline bool check_permission(PTE *pte, bool ok, vaddr_t vaddr, int type) {
  bool ifetch = (type == MEM_TYPE_IFETCH);
  // uint32_t mode = (mstatus->mprv && !ifetch ? mstatus->mpp : cpu.mode);
  // we don't use mprv now
  uint32_t mode = cpu.mode;
  assert(mode == MODE_U || mode == MODE_S);
  ok = ok && pte->valid;
  ok = ok && !(mode == MODE_U && !pte->user);
  // Logtr("ok: %i, mode == U: %i, pte->u: %i, ppn: %x", ok, mode == MODE_U, pte->u, (uint64_t)pte->ppn << 12);
  // ok = ok && !(pte->u && ((mode == MODE_S) && (!mstatus->sum || ifetch)));
  // we don't use sum now
  if (ifetch) {
    Logtr("Translate for instr reading");
#ifdef CONFIG_SHARE
//  update a/d by exception
    bool update_ad = !pte->access;
    if (update_ad && ok && pte->exec)
      Logtr("raise exception to update ad for ifecth");
#else
    bool update_ad = false;
#endif
    if (!(ok && pte->exec) || update_ad) {
      // assert(!cpu.amo);
      INTR_TVAL_REG(EX_IPF) = vaddr;
      longjmp_exception(EX_IPF);
      return false;
    }
  } else if (type == MEM_TYPE_READ) {
    Logtr("Translate for memory reading");
    // bool can_load = pte->read || (mstatus->mxr && pte->x); we don't use mxr now
    bool can_load = pte->read;
#ifdef CONFIG_SHARE
    bool update_ad = !pte->access;
    if (update_ad && ok && can_load)
      Logtr("raise exception to update ad for load");
#else
    bool update_ad = false;
#endif
    if (!(ok && can_load) || update_ad) {
      // if (cpu.amo) Logtr("redirect to AMO page fault exception at pc = " FMT_WORD, cpu.pc);
      int ex = EX_LPF;
      INTR_TVAL_REG(ex) = vaddr;
      // cpu.amo = false;
      Logtr("Memory read translation exception!");
      longjmp_exception(ex);
      return false;
    }
  } else {
#ifdef CONFIG_SHARE
    bool update_ad = !pte->access || !pte->dirty;
   if (update_ad && ok && pte->write) Logtr("raise exception to update ad for store");
#else
    bool update_ad = false;
#endif
    Logtr("Translate for memory writing");
    if (!(ok && pte->write) || update_ad) {
      INTR_TVAL_REG(EX_SPF) = vaddr;
      longjmp_exception(EX_SPF);
      return false;
    }
  }
  return true;
}

static paddr_t ptw(vaddr_t vaddr, int type) {
  Logtr("Page walking for 0x%x\n", vaddr);
  word_t pg_base = PGBASE(satp->ppn);
  word_t p_pte; // pte pointer
  PTE pte;
  int level;
  for (level = PTW_LEVEL - 1; level >= 0;) {
    p_pte = pg_base + VPNi(vaddr, level) * PTE_SIZE;
#ifdef CONFIG_MULTICORE_DIFF
    pte.val = golden_pmem_read(p_pte, PTE_SIZE, 0, 0, 0);
#else
    pte.val	= paddr_read(p_pte, PTE_SIZE,
      type == MEM_TYPE_IFETCH ? MEM_TYPE_IFETCH_READ :
      type == MEM_TYPE_WRITE ? MEM_TYPE_WRITE_READ : MEM_TYPE_READ, MODE_S, vaddr);
#endif
#ifdef CONFIG_SHARE
    if (unlikely(dynamic_config.debug_difftest)) {
      fprintf(stderr, "[NEMU] ptw: level %d, vaddr 0x%x, pg_base 0x%x, p_pte 0x%x, pte.val 0x%x\n",
        level, vaddr, pg_base, p_pte, pte.val);
    }
#endif
    pg_base = PGBASE(pte.ppn);
    if (!pte.valid || (!pte.read && pte.write)) goto bad;
    if (pte.read || pte.exec) { break; }
    else {
      level --;
      if (level < 0) { goto bad; }
    }
  }

  if (!check_permission(&pte, true, vaddr, type)) return MEM_RET_FAIL;

  if (level > 0) {
    // superpage
    word_t pg_mask = ((1ull << VPNiSHFT(level)) - 1);
    if ((pg_base & pg_mask) != 0) {
      // missaligned superpage
      goto bad;
    }
    pg_base = (pg_base & ~pg_mask) | (vaddr & pg_mask & ~PGMASK);
  }

// #ifndef CONFIG_SHARE
//   // update a/d by hardware
//   bool is_write = (type == MEM_TYPE_WRITE);
//   if (!pte.a || (!pte.d && is_write)) {
//     pte.a = true;
//     pte.d |= is_write;
//     paddr_write(p_pte, PTE_SIZE, pte.val, cpu.mode, vaddr);
//   }
// #endif

  return pg_base | MEM_RET_OK;

bad:
  Logtr("Memory translation bad");
  check_permission(&pte, false, vaddr, type);
  return MEM_RET_FAIL;
}

// static paddr_t ptw(vaddr_t vaddr, int type) {
//   uint32_t vpn = vaddr >> 12;
//   int idx = TLB_hash(vpn);
//   TLBEntry *e = &TLB[idx];
//   if (e->vpn == vpn && TLBValid[idx]) {
//   //  if (cpu.pc == 0x40000120) Log("vaddr = 0x%x, ppage = 0x%x", vaddr, e->ppage);
//    return e->ppage;
//   }

//   PageAddr *addr = (void *)&vaddr;
//   paddr_t pdir_base = satp->ppn << 12;

//   PTE pde;
//   pde.val	= paddr_read(pdir_base + addr->pdir_idx * 4, 4, MEM_TYPE_READ, MODE_S, vaddr);
//   if (!pde.valid) {
//     panic("pc = %x, vaddr = %x, pdir_base = %x, pde = %x", cpu.pc, vaddr, pdir_base, pde.val);
//   }

//   paddr_t pt_base = pde.ppn << 12;
//   PTE pte;
//   pte.val = paddr_read(pt_base + addr->pt_idx * 4, 4, MEM_TYPE_READ, MODE_S, vaddr);
//   if (!pte.valid) {
//     panic("pc = %x, vaddr = %x, pt_base = %x, pte = %x", cpu.pc, vaddr, pt_base, pte.val);
//   }

//   // update TLB
//   *e = (TLBEntry) { .vpn = vpn, .ppage = pte.ppn << 12 };
//   TLBValid[idx] = true;

//   //bool is_write = (type == MEM_TYPE_WRITE);
//   //if (!pte.access || (pte.dirty == 0 && is_write)) {
//   //  pte.access = 1;
//   //  pte.dirty |= is_write;
//   //  paddr_write(pt_base + addr->pt_idx * 4, 4, pte.val, cpu.mode);
//   //}

//   return pte.ppn << 12;
// }

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  return ptw(vaddr, type) | MEM_RET_OK;
}
#else
paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  return MEM_RET_FAIL;
}
#endif

bool isa_pmp_check_permission(paddr_t addr, int len, int type, int mode) {
  return true; // don't need pmp
}