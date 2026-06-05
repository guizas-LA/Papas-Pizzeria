/**
 * @file draw_utils.h
 * @brief Low-level drawing primitives: frame-buffer management, shapes, font, and buttons.
 */

#ifndef PROJECT_DRAW_UTILS_H
#define PROJECT_DRAW_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <lcom/lcf.h>

/**
 * @brief Initialises the custom frame-buffer by mapping the VBE linear frame-buffer and
 *        allocating a matching back-buffer used for double-buffered rendering.
 * @param mode VBE mode number (e.g. @c GAME_VIDEO_MODE).
 * @return 0 on success, 1 on failure.
 */
int draw_init(uint16_t mode);

/**
 * @brief Fills the entire back-buffer with a solid colour.
 * @param color Packed 24-bit RGB colour produced by @c rgb().
 */
void draw_clear(uint32_t color);

/**
 * @brief Draws a filled axis-aligned rectangle, clipped to screen bounds.
 * @param x     Left edge in pixels.
 * @param y     Top edge in pixels.
 * @param w     Width in pixels.
 * @param h     Height in pixels.
 * @param color Fill colour.
 */
void draw_rect(int x, int y, int w, int h, uint32_t color);

/**
 * @brief Draws a filled circle, clipped to screen bounds.
 * @param cx     Centre x in pixels.
 * @param cy     Centre y in pixels.
 * @param radius Radius in pixels.
 * @param color  Fill colour.
 */
void draw_circle(int cx, int cy, int radius, uint32_t color);

/**
 * @brief Sets a single pixel in the back-buffer.
 * @param x     Horizontal position.
 * @param y     Vertical position.
 * @param color Pixel colour.
 */
void draw_pixel(int x, int y, uint32_t color);

/**
 * @brief Blits an XPM pixmap at the given screen position, clipped to screen bounds.
 * @param pixmap Pointer to the decoded pixel data.
 * @param img    XPM image metadata (width, height).
 * @param x      Left edge of the destination rectangle.
 * @param y      Top edge of the destination rectangle.
 */
void draw_xpm(uint8_t *pixmap, xpm_image_t img, int x, int y);

/**
 * @brief Blits an XPM pixmap scaled to fill the given destination dimensions,
 *        starting at (0, 0) on screen.
 * @param pixmap Pointer to the decoded pixel data.
 * @param img    XPM image metadata.
 * @param dst_w  Destination width in pixels.
 * @param dst_h  Destination height in pixels.
 */
void draw_xpm_scaled(uint8_t *pixmap, xpm_image_t img, int dst_w, int dst_h);

/**
 * @brief Copies the back-buffer to the hardware frame-buffer (page flip).
 */
void draw_swap(void);

/**
 * @brief Packs three 8-bit channel values into a single 24-bit colour word.
 * @param r Red component (0–255).
 * @param g Green component (0–255).
 * @param b Blue component (0–255).
 * @return Packed colour as @c 0x00RRGGBB.
 */
uint32_t rgb(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Renders a single character using the built-in bitmap font.
 * @param x     Left edge of the character cell.
 * @param y     Top edge of the character cell.
 * @param c     ASCII character to draw.
 * @param scale Pixel scale factor (1 = normal, 2 = double, …).
 * @param color Glyph colour.
 */
void draw_char(int x, int y, char c, int scale, uint32_t color);

/**
 * @brief Renders a null-terminated ASCII/UTF-8 string with the built-in bitmap font.
 *        Handles the accented characters encoded in the font (Á, À, Ç, Ã, Õ, +).
 * @param x     Starting x position.
 * @param y     Starting y position.
 * @param s     Null-terminated string to draw.
 * @param scale Pixel scale factor.
 * @param color Glyph colour.
 */
void draw_string(int x, int y, const char *s, int scale, uint32_t color);

/**
 * @brief Draws a framed panel: a solid outer border with a light cream inner fill.
 * @param x     Left edge.
 * @param y     Top edge.
 * @param w     Width.
 * @param h     Height.
 * @param color Border colour.
 */
void draw_panel(int x, int y, int w, int h, uint32_t color);

/**
 * @brief Draws an interactive button using the sprite system (with fallback to rectangles).
 *        The border colour changes to indicate the selected / highlighted state.
 * @param x        Left edge.
 * @param y        Top edge.
 * @param w        Width.
 * @param h        Height.
 * @param selected @c true when the button is highlighted (keyboard cursor or mouse hover).
 * @param color    Button fill colour.
 */
void draw_button(int x, int y, int w, int h, bool selected, uint32_t color);

#endif
