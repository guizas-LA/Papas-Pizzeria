/**
 * @file draw_elements.h
 * @brief Declarations for game-specific composite drawing helpers.
 */

#ifndef PROJECT_DRAW_ELEMENTS_H
#define PROJECT_DRAW_ELEMENTS_H

#include <stdbool.h>
#include <stdint.h>
#include "game_state.h"

/** @brief RGB fill colours for each of the six topping types. */
extern const uint32_t TOPPING_COLORS[6];

/**
 * @brief Draws a filled diamond (rotated square) shape.
 * @param cx    Centre x coordinate.
 * @param cy    Centre y coordinate.
 * @param size  Half-diagonal size in pixels.
 * @param color Fill colour.
 */
void draw_diamond(int cx, int cy, int size, uint32_t color);

/**
 * @brief Draws the order ticket panel, switching between the order view and the
 *        delivery result view depending on @c game->playing_state.
 * @param game Pointer to the current game state.
 * @param x    Left edge of the ticket panel in pixels.
 * @param y    Top edge of the ticket panel in pixels.
 */
void draw_order_ticket(Game *game, int x, int y);

/**
 * @brief Draws a horizontally and vertically centred text label inside a button rectangle.
 * @param bx    Button left edge.
 * @param by    Button top edge.
 * @param bw    Button width.
 * @param bh    Button height.
 * @param s     Null-terminated label string.
 * @param scale Pixel scale factor for the font.
 * @param color Text colour.
 */
void draw_button_label(int bx, int by, int bw, int bh, const char *s, int scale, uint32_t color);

/**
 * @brief Returns whether topping type @p t is the currently active placement tool.
 * @param game Pointer to the current game state.
 * @param t    Topping type index (0–5).
 * @return @c true if @c game->active_topping equals @p t.
 */
bool topping_selected(Game *game, int t);

/**
 * @brief Draws the pizza with its sauce, cheese layer, and individual topping dots.
 *        Darkens the pizza progressively when it is overbaked.
 * @param game Pointer to the current game state.
 * @param cx   Centre x of the pizza circle.
 * @param cy   Centre y of the pizza circle.
 * @param r    Radius of the pizza in pixels.
 */
void draw_pizza(Game *game, int cx, int cy, int r);

/**
 * @brief Draws all cut lines and the rim dots used for cut-point selection.
 * @param game Pointer to the current game state.
 * @param cx   Centre x of the pizza circle.
 * @param cy   Centre y of the pizza circle.
 * @param r    Radius of the pizza in pixels.
 */
void draw_pizza_cuts(Game *game, int cx, int cy, int r);

#endif
