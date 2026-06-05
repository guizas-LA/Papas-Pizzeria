/**
 * @file keyboard_logic.h
 * @brief Declaration of the keyboard interrupt handler.
 */

#pragma once
#include "game_state.h"

/**
 * @brief Processes a single PS/2 Set-1 scancode and updates game state accordingly.
 *        Handles ESC, ENTER, arrow keys, letter keys (for name entry), and digit keys.
 * @param game     Pointer to the current game state.
 * @param scancode Raw PS/2 Set-1 scancode byte received from the KBC.
 */
void game_handle_keyboard(Game *game, uint8_t scancode);
