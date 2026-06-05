/**
 * @file main.c
 * @brief LCOM framework entry point; delegates immediately to the game loop.
 */

#include <lcom/lcf.h>

#include "loop.h"

/**
 * @brief Standard C entry point; initialises the LCOM framework.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char *argv[]) {
  lcf_set_language("EN-US");
  lcf_trace_calls("/home/lcom/labs/project/trace.txt");
  lcf_log_output("/home/lcom/labs/project/output.txt");

  if (lcf_start(argc, argv)) return 1;

  lcf_cleanup();
  return 0;
}

/**
 * @brief LCOM-required entry point called after framework initialisation.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Return value of @c game_loop().
 */
int(proj_main_loop)(int argc, char *argv[]) {
  return game_loop(argc, argv);
}
