/**
 * @file loop.h
 * @brief Declaration of the main game loop entry point.
 */

#ifndef PROJECT_LOOP_H
#define PROJECT_LOOP_H

/**
 * @brief Initialises all subsystems and runs the game event loop until the game exits.
 * @param argc Argument count forwarded from @c proj_main_loop.
 * @param argv Argument vector forwarded from @c proj_main_loop.
 * @return 0 on clean exit, 1 on initialisation failure.
 */
int game_loop(int argc, char *argv[]);

#endif
