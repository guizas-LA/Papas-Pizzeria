/**
 * @file mouse_logic.h
 * @brief Declarations for mouse click and packet processing.
 */

#pragma once
#include "game_state.h"

/**
 * @brief Processes a pending left-click by testing hit regions for every game state.
 *        Clears @c game->mouse_left_click when done.
 * @param game Pointer to the current game state.
 */
void handle_click(Game *game);

/**
 * @brief Updates the cursor position and detects rising-edge left-click events.
 * @param game   Pointer to the current game state.
 * @param packet Decoded mouse packet from the PS/2 controller.
 */
void game_handle_mouse_packet(Game *game, struct packet *packet);
