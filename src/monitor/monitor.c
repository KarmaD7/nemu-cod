#include <isa.h>
#include <memory/image_loader.h>
#include <memory/paddr.h>
#include <getopt.h>
#include <stdlib.h>

#ifndef CONFIG_SHARE
void init_aligncheck();
void init_log(const char *log_file);
void init_mem();
void init_regex();
void init_wp_pool();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static int batch_mode = false;
static int difftest_port = 1234;
char *max_instr = NULL;

int is_batch_mode() { return batch_mode; }

static inline void welcome() {
  Log("Debug: \33[1;32m%s\33[0m", MUXDEF(CONFIG_DEBUG, "ON","OFF"));
  IFDEF(CONFIG_DEBUG, Log("If debug mode is on, a log file will be generated "
      "to record every instruction NEMU executes. This may lead to a large log file. "
      "If it is not necessary, you can turn it off in include/common.h.")
  );
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to \33[1;41m\33[1;33m%s\33[0m-NEMU!\n", str(__ISA__));
  printf("For help, type \"help\"\n");
}

static inline int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"max-instr", required_argument, NULL, 'I'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},

    // path setting
    {"output-base-dir"    , required_argument, NULL, 'D'},
    {"workload-name"      , required_argument, NULL, 'w'},
    {"config-name"        , required_argument, NULL, 'C'},

    // restore cpt
    {"restore"            , no_argument      , NULL, 'c'},
    {"cpt-restorer"       , required_argument, NULL, 'r'},

    // take cpt
    {"simpoint-dir"       , required_argument, NULL, 'S'},
    {"uniform-cpt"        , no_argument      , NULL, 'u'},
    {"cpt-interval"       , required_argument, NULL, 5},
    {"cpt-mmode"          , no_argument      , NULL, 7},

    // profiling
    {"simpoint-profile"   , no_argument      , NULL, 3},
    {"dont-skip-boot"     , no_argument      , NULL, 6},

    // restore cpt
    {"cpt-id"             , required_argument, NULL, 4},

    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bI:hl:d:p:D:w:C:cr:S:u", table, NULL)) != -1) {
    switch (o) {
      case 'b': batch_mode = true; break;
      case 'I': max_instr = optarg; break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 1: img_file = optarg; return optind - 1;

      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-I,--max-instr          max number of instructions executed\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");

        printf("\t-D,--statdir=STAT_DIR   store simpoint bbv, cpts, and stats in STAT_DIR\n");
        printf("\t-w,--workload=WORKLOAD  the name of sub_dir of this run in STAT_DIR\n");
        printf("\t-C,--config=CONFIG      running configuration\n");

        printf("\t-c,--restore            restoring from CPT FILE\n");
        printf("\t-r,--cpt-restorer=R     binary of gcpt restorer\n");

        printf("\t-S,--simpoint-dir=SIMPOINT_DIR   simpoints dir\n");
        printf("\t-u,--uniform-cpt        uniformly take cpt with fixed interval\n");
        printf("\t--cpt-interval=INTERVAL cpt interval: the profiling period for simpoint; the checkpoint interval for uniform cpt\n");
        printf("\t--cpt-mmode             force to take cpt in mmode, which might not work.\n");

        printf("\t--simpoint-profile      simpoint profiling\n");
        printf("\t--dont-skip-boot        profiling/checkpoint immediately after boot\n");
        printf("\t--cpt-id                checkpoint id\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
#ifdef CONFIG_MODE_USER
  int user_argidx = parse_args(argc, argv);
#else
  parse_args(argc, argv);
#endif
  
  /* Open the log file. */
  init_log(log_file);

  /* Initialize memory. */
  init_mem();

  /* Load the image to memory. This will overwrite the built-in image. */
#ifdef CONFIG_MODE_USER
  int user_argc = argc - user_argidx;
  char **user_argv = argv + user_argidx;
  void init_user(char *elfpath, int argc, char *argv[]);
  init_user(img_file, user_argc, user_argv);
#else
  /* Perform ISA dependent initialization. */
  init_isa();

  // when there is a gcpt[restorer], we put bbl after gcpt[restorer]
  uint64_t bbl_start;
  long img_size; // how large we should copy for difftest

  bbl_start = RESET_VECTOR;
  img_size = load_img(img_file, "image (bbl/bare metal app) from cmdline", bbl_start, 0);

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize devices. */
  init_device();
#endif

  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();

  /* Enable alignment checking for in a x86 host */
  init_aligncheck();

  /* Display welcome message. */
  welcome();
}
#endif
