/**
 * @file draw.h
 * @brief Declaration of the main frame-rendering function.
 */

#ifndef PROJECT_DRAW_H
#define PROJECT_DRAW_H

#include "game_state.h"

/**
 * @brief Renders the full game frame for the current state into the back-buffer and swaps it.
 * @param game Pointer to the current game state (read-only for rendering).
 */
void game_draw(Game *game);

#endif
