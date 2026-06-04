#include <lcom/lcf.h>

#include "loop.h"

int main(int argc, char *argv[]) {
  lcf_set_language("EN-US");
  lcf_trace_calls("/home/lcom/labs/project/trace.txt");
  lcf_log_output("/home/lcom/labs/project/output.txt");

  if (lcf_start(argc, argv)) return 1;

  lcf_cleanup();
  return 0;
}

int(proj_main_loop)(int argc, char *argv[]) {
  return game_loop(argc, argv);
}
