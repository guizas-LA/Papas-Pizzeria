/**
 * @file game_logic.h
 * @brief Declarations for core gameplay logic: order generation, oven, scoring, and delivery.
 */

#pragma once
#include "game_state.h"

/**
 * @brief Generates a new customer order and resets all preparation state.
 * @param game Pointer to the current game state.
 */
void make_order(Game *game);

/**
 * @brief Checks whether the pizza has been in the oven for the required duration.
 * @param game Pointer to the current game state.
 * @return @c true if the oven has reached the target tick count.
 */
bool oven_ready(Game *game);

/**
 * @brief Starts the oven, resetting the oven tick counter.
 * @param game Pointer to the current game state.
 */
void start_oven(Game *game);

/**
 * @brief Finalises the current order, updates day totals, and advances to the next order
 *        or to the closed-screen state when the day's order quota is met.
 * @param game Pointer to the current game state.
 */
void serve_pizza(Game *game);

/**
 * @brief Validates the typed customer name and, if correct, scores and delivers the pizza.
 *        Transitions to @c PLAYING_DELIVERED regardless of name correctness.
 * @param game Pointer to the current game state.
 */
void try_deliver(Game *game);

/**
 * @brief Selects or deselects the given topping type as the active placement tool.
 * @param game Pointer to the current game state.
 * @param t    Topping type index (0–5).
 */
void toggle_topping(Game *game, int t);

/**
 * @brief Reads the current RTC time and stores it in @c game->order_time_str.
 * @param game Pointer to the current game state.
 */
void record_order_start(Game *game);
