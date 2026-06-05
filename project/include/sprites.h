/**
 * @file sprites.h
 * @brief Sprite management: loading, destruction, and drawing with template colourisation.
 */

#ifndef PROJECT_SPRITES_H
#define PROJECT_SPRITES_H

#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Stores a decoded XPM sprite as a flat array of 32-bit ARGB pixels.
 */
typedef struct {
  uint32_t *colors; /**< Pixel data array (row-major, width × height entries). */
  uint16_t width;   /**< Image width in pixels. */
  uint16_t height;  /**< Image height in pixels. */
} Sprite;

extern Sprite *buttonSprite;        /**< Normal (unselected) button template sprite. */
extern Sprite *buttonPressedSprite; /**< Pressed (selected) button template sprite. */
extern Sprite *mouseCursorSprite;   /**< Mouse cursor sprite. */
extern Sprite *orderTicketSprite;   /**< Order ticket background sprite. */
extern Sprite *openSignSprite;      /**< "Open" sign sprite shown during TAKE_ORDER. */
extern Sprite *closedSignSprite;    /**< "Closed" sign sprite shown on the last order. */

/**
 * @brief Allocates a @c Sprite and loads pixel data from an XPM map.
 * @param sprite XPM source map (as returned by the XPM include).
 * @return Pointer to the new sprite, or @c NULL on failure.
 */
Sprite *createSprite(xpm_map_t sprite);

/**
 * @brief Frees the pixel buffer and the sprite struct itself.
 * @param sprite Pointer to the sprite to destroy (may be @c NULL).
 */
void destroy_sprite(Sprite *sprite);

/**
 * @brief Blits a sprite at the given position, skipping transparent pixels.
 * @param sprite Pointer to the sprite to draw.
 * @param x      Destination x coordinate.
 * @param y      Destination y coordinate.
 * @return 0 on success, 1 if the sprite is @c NULL.
 */
int drawSprite(Sprite *sprite, int x, int y);

/**
 * @brief Blits a sprite scaled to the given dimensions using nearest-neighbour sampling.
 * @param sprite Pointer to the sprite to draw.
 * @param x      Destination x coordinate.
 * @param y      Destination y coordinate.
 * @param width  Destination width in pixels.
 * @param height Destination height in pixels.
 * @return 0 on success, 1 if the sprite is @c NULL or dimensions are invalid.
 */
int drawSpriteScaled(Sprite *sprite, int x, int y, int width, int height);

/**
 * @brief Draws a colourised button using the template sprites (or falls back to rectangles).
 *        Template pixels are replaced with a shade of @p color; other colours are kept.
 * @param x        Destination x coordinate.
 * @param y        Destination y coordinate.
 * @param width    Button width in pixels.
 * @param height   Button height in pixels.
 * @param selected @c true uses the pressed-button template at 75% brightness.
 * @param color    Base fill colour applied to the template region.
 * @return 0 on success, 1 if sprite data is unavailable.
 */
int drawButtonSprite(int x, int y, int width, int height, bool selected, uint32_t color);

/**
 * @brief Draws the mouse cursor sprite at the given position.
 * @param x Cursor x coordinate.
 * @param y Cursor y coordinate.
 * @return 0 on success, 1 if the sprite is not loaded.
 */
int drawMouseCursorSprite(int x, int y);

/**
 * @brief Draws the order ticket sprite scaled to the given dimensions.
 * @param x      Destination x coordinate.
 * @param y      Destination y coordinate.
 * @param width  Destination width in pixels.
 * @param height Destination height in pixels.
 * @return 0 on success, 1 if the sprite is not loaded.
 */
int drawOrderTicketSprite(int x, int y, int width, int height);

/**
 * @brief Draws the "open" sign sprite scaled to the given dimensions.
 * @param x      Destination x coordinate.
 * @param y      Destination y coordinate.
 * @param width  Destination width in pixels.
 * @param height Destination height in pixels.
 * @return 0 on success, 1 if the sprite is not loaded.
 */
int drawOpenSignSprite(int x, int y, int width, int height);

/**
 * @brief Draws the "closed" sign sprite scaled to the given dimensions.
 * @param x      Destination x coordinate.
 * @param y      Destination y coordinate.
 * @param width  Destination width in pixels.
 * @param height Destination height in pixels.
 * @return 0 on success, 1 if the sprite is not loaded.
 */
int drawClosedSignSprite(int x, int y, int width, int height);

/**
 * @brief Loads all game sprites from their embedded XPM sources.
 * @return 0 on success, 1 if any sprite fails to load.
 */
int loadSprites(void);

/**
 * @brief Destroys all loaded sprites and resets their global pointers to @c NULL.
 */
void unloadSprites(void);

#endif
