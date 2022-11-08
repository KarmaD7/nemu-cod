#include <cpu/cpu.h>
#include <cpu/exec.h>
#include <cpu/difftest.h>
#include <cpu/decode.h>
#include <memory/host-tlb.h>
#include <isa-all-instr.h>
#include <locale.h>
#include <setjmp.h>
#include <unistd.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INSTR_TO_PRINT 10
#ifndef CONFIG_SHARE
#define BATCH_SIZE 65536
#else
#define BATCH_SIZE 1
#endif

CPU_state cpu = {};
uint64_t g_nr_guest_instr = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;
const rtlreg_t rzero = 0;
rtlreg_t tmp_reg[4];

#ifdef CONFIG_DEBUG
static inline void debug_hook(vaddr_t pc, const char *asmbuf) {
  log_write("%s\n", asmbuf);
  if (g_print_step) { puts(asmbuf); }

  void scan_watchpoint(vaddr_t pc);
  scan_watchpoint(pc);
}
#endif

static jmp_buf jbuf_exec = {};
static uint64_t n_remain_total;
static int n_remain;
static Decode *prev_s;

void save_globals(Decode *s) {
  IFDEF(CONFIG_PERF_OPT, prev_s = s);
}

uint64_t get_abs_instr_count () {
#if defined(CONFIG_ENABLE_INSTR_CNT)
  int n_batch = n_remain_total >= BATCH_SIZE ? BATCH_SIZE : n_remain_total;
  uint32_t n_executed = n_batch - n_remain;
  return n_executed + g_nr_guest_instr;
#endif
  return 0;
}

static void update_instr_cnt() {
#if defined(CONFIG_ENABLE_INSTR_CNT)
  int n_batch = n_remain_total >= BATCH_SIZE ? BATCH_SIZE : n_remain_total;
  uint32_t n_executed = n_batch - n_remain;
  n_remain_total -= (n_remain_total > n_executed) ? n_executed : n_remain_total;
  IFNDEF(CONFIG_DEBUG, g_nr_guest_instr += n_executed);

  n_remain = n_batch > n_remain_total ? n_remain_total : n_batch; // clean n_remain
  // Loge("n_remain = %i, n_remain_total = %lu\n", n_remain, n_remain_total);
#endif
}

void monitor_statistic() {
  update_instr_cnt();
  setlocale(LC_NUMERIC, "");
  Log("host time spent = %'ld us", g_timer);
#ifdef CONFIG_ENABLE_INSTR_CNT
  Log("total guest instructions = %'ld", g_nr_guest_instr);
  if (g_timer > 0) Log("simulation frequency = %'ld instr/s", g_nr_guest_instr * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
#else
  Log("CONFIG_ENABLE_INSTR_CNT is not defined");
#endif
}

static word_t g_ex_cause = 0;
static int g_sys_state_flag = 0;

void set_sys_state_flag(int flag) {
  g_sys_state_flag |= flag;
}

void mmu_tlb_flush(vaddr_t vaddr) {
  hosttlb_flush(vaddr);
  if (vaddr == 0) set_sys_state_flag(SYS_STATE_FLUSH_TCACHE);
}

void longjmp_exec(int cause) {
  Loge("Longjmp to jbuf_exec with cause: %i", cause);
  longjmp(jbuf_exec, cause);
}

void longjmp_exception(int ex_cause) {
#ifdef CONFIG_GUIDED_EXEC
  cpu.guided_exec = false;
#endif
  g_ex_cause = ex_cause;
  Loge("longjmp_exec(NEMU_EXEC_EXCEPTION)");
  longjmp_exec(NEMU_EXEC_EXCEPTION);
}

#define FILL_EXEC_TABLE(name) [concat(EXEC_ID_, name)] = concat(exec_, name),

#define rtl_priv_next(s)
#define rtl_priv_jr(s, target) rtl_jr(s, target)

#include "isa-exec.h"
static const void* g_exec_table[TOTAL_INSTR] = {
  MAP(INSTR_LIST, FILL_EXEC_TABLE)
};

static int execute(int n) {
  static Decode s;
  prev_s = &s;
  for (;n > 0; n --) {
    fetch_decode(&s, cpu.pc);
    // cpu.debug.current_pc = s.pc;
    cpu.pc = s.snpc;
#ifdef CONFIG_SHARE
    if (unlikely(dynamic_config.debug_difftest)) {
      fprintf(stderr, "(%d) [NEMU] pc = 0x%x inst %x\n", getpid(), s.pc, s.isa.instr.val);
    }
#endif
    s.EHelper(&s);
    g_nr_guest_instr ++;
    IFDEF(CONFIG_DEBUG, debug_hook(s.pc, s.logbuf));
    IFDEF(CONFIG_DIFFTEST, difftest_step(s.pc, cpu.pc));
  }
  return n;
}


void fetch_decode(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  IFDEF(CONFIG_DEBUG, log_bytebuf[0] = '\0');
  int idx = isa_fetch_decode(s);
  Logtid(FMT_WORD ":   %s%*.s%s",
        s->pc, log_bytebuf, 40 - (12 + 3 * (int)(s->snpc - s->pc)), "", log_asmbuf);
  IFDEF(CONFIG_DEBUG, snprintf(s->logbuf, sizeof(s->logbuf), FMT_WORD ":   %s%*.s%s",
        s->pc, log_bytebuf, 40 - (12 + 3 * (int)(s->snpc - s->pc)), "", log_asmbuf));
  s->EHelper = g_exec_table[idx];
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  printf("Entering cpu exec... \n");
  IFDEF(CONFIG_SHARE, assert(n <= 1));
  g_print_step = (n < MAX_INSTR_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default:
      nemu_state.state = NEMU_RUNNING;
      Loge("Setting NEMU state to RUNNING");
  }

  uint64_t timer_start = get_time();

  n_remain_total = n; // deal with setjmp()
  Loge("cpu_exec will exec %lu instrunctions", n_remain_total);
  int cause;
  if ((cause = setjmp(jbuf_exec))) {
    n_remain -= prev_s->idx_in_bb - 1;
    // Here is exception handle
    Loge("After update_global, n_remain: %i, n_remain_total: %li", n_remain, n_remain_total);
  }

  while (nemu_state.state == NEMU_RUNNING &&
      MUXDEF(CONFIG_ENABLE_INSTR_CNT, n_remain_total > 0, true)) {
// #ifdef CONFIG_DEVICE
//     extern void device_update();
//     device_update();
// #endif

    if (cause == NEMU_EXEC_EXCEPTION) {
      Loge("Handle NEMU_EXEC_EXCEPTION");
      cause = 0;
      cpu.pc = raise_intr(g_ex_cause, prev_s->pc);
      IFDEF(CONFIG_PERF_OPT, tcache_handle_exception(cpu.pc));
      IFDEF(CONFIG_SHARE, break);
    } else {
      word_t intr = MUXDEF(CONFIG_SHARE, INTR_EMPTY, isa_query_intr());
      if (intr != INTR_EMPTY) {
        Loge("NEMU raise intr");
        cpu.pc = raise_intr(intr, cpu.pc);
        IFDEF(CONFIG_DIFFTEST, ref_difftest_raise_intr(intr));
        IFDEF(CONFIG_PERF_OPT, tcache_handle_exception(cpu.pc));
      }
    }

    int n_batch = n_remain_total >= BATCH_SIZE ? BATCH_SIZE : n_remain_total;
    n_remain = execute(n_batch);
#ifdef CONFIG_PERF_OPT
    // return from execute
    update_global(cpu.pc);
    Loge("n_remain_total: %lu", n_remain_total);
#else
    n_remain_total -= n_batch;
#endif
  }

  // If nemu_state.state is NEMU_RUNNING, n_remain_total should be zero.
  if (nemu_state.state == NEMU_RUNNING) {
    nemu_state.state = NEMU_QUIT;
  }

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP;
      Loge("NEMU stopped when running");
      if (ISDEF(CONFIG_EXITLOG)) {
        monitor_statistic();
      }
      break;

    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s\33[0m at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? "\33[1;31mABORT" :
           (nemu_state.halt_ret == 0 ? "\33[1;32mHIT GOOD TRAP" : "\33[1;31mHIT BAD TRAP")),
          nemu_state.halt_pc);
      Log("trap code:%d", nemu_state.halt_ret);
      monitor_statistic();
      break;
    case NEMU_QUIT:
#ifndef CONFIG_SHARE
      monitor_statistic();
#else
      break;
#endif
    }
}
