/**
 * @file sprites.c
 * @brief XPM-based sprite system: loading, destruction, and template-colourised drawing.
 */

#include "sprites.h"

#include <stdlib.h>

#include "button_sprites.xpm"
#include "draw_utils.h"
#include "graphics.h"
#include "order_ticket.xpm"
#include "open_sign.xpm"
#include "closed_sign.xpm"

#pragma clang optimize off

/** @brief Colour value used to mark transparent pixels in sprites. */
#define TRANSPARENT_RGB       0x00FFFFFE
/** @brief Template colour marking the main fill region of a normal button. */
#define TEMPLATE_FILL         0x0000ED2F
/** @brief Template colour marking the main fill region of a pressed button. */
#define TEMPLATE_PRESSED_FILL 0x00027A18
/** @brief Template colour marking the highlight (bright edge) of a button. */
#define TEMPLATE_HIGHLIGHT    0x00FFFFFF
/** @brief Template colour marking the border of a button. */
#define TEMPLATE_BORDER       0x00000000

#define SCREEN_W 800 /**< Screen width in pixels. */
#define SCREEN_H 600 /**< Screen height in pixels. */

Sprite *buttonSprite        = NULL; /**< Normal button template sprite. */
Sprite *buttonPressedSprite = NULL; /**< Pressed button template sprite. */
Sprite *mouseCursorSprite   = NULL; /**< Mouse cursor sprite. */
Sprite *orderTicketSprite   = NULL; /**< Order ticket background sprite. */
Sprite *openSignSprite      = NULL; /**< "Open" sign sprite. */
Sprite *closedSignSprite    = NULL; /**< "Closed" sign sprite. */

/**
 * @brief Strips the alpha byte from a 32-bit ARGB value, returning only RGB.
 * @param color Input 32-bit colour value.
 * @return 24-bit RGB value (bits 23–0).
 */
static uint32_t sprite_rgb(uint32_t color) {
  return color & 0x00FFFFFF;
}

/**
 * @brief Writes a single pixel via @c draw_pixel(), clipped to screen bounds.
 * @param x     Horizontal position.
 * @param y     Vertical position.
 * @param color Pixel colour.
 * @return Always 0 (clips silently rather than reporting out-of-bounds as an error).
 */
static int draw_clipped_pixel(int x, int y, uint32_t color) {
  if (x < 0 || y < 0 || x >= SCREEN_W || y >= SCREEN_H) return 0;
  draw_pixel(x, y, color);
  return 0;
}

/**
 * @brief Scales each RGB component of a colour by a given percentage.
 * @param color   Source colour.
 * @param percent Scale factor (100 = unchanged, 75 = 75% brightness, 125 = brightened).
 * @return Scaled colour, clamped to 255 per channel.
 */
static uint32_t shade_color(uint32_t color, uint8_t percent) {
  unsigned r = ((color >> 16) & 0xFF) * percent / 100;
  unsigned g = ((color >> 8) & 0xFF) * percent / 100;
  unsigned b = (color & 0xFF) * percent / 100;

  if (r > 255) r = 255;
  if (g > 255) g = 255;
  if (b > 255) b = 255;

  return rgb(r, g, b);
}

/**
 * @brief Allocates a @c Sprite and loads its pixel data from an XPM map using ARGB mode.
 * @param sprite XPM source map.
 * @return Pointer to the new sprite on success, @c NULL on allocation or load failure.
 */
Sprite *createSprite(xpm_map_t sprite) {
  xpm_image_t img;
  Sprite *sp = (Sprite *) malloc(sizeof(Sprite));

  if (sp == NULL) return NULL;

  sp->colors = (uint32_t *) xpm_load(sprite, XPM_8_8_8_8, &img);
  if (sp->colors == NULL) {
    free(sp);
    return NULL;
  }

  sp->height = img.height;
  sp->width = img.width;

  return sp;
}

/**
 * @brief Frees the pixel buffer and the sprite struct itself.
 * @param sprite Pointer to the sprite (may be @c NULL).
 */
void destroy_sprite(Sprite *sprite) {
  if (sprite == NULL) return;

  if (sprite->colors != NULL) {
    free(sprite->colors);
  }

  free(sprite);
}

/**
 * @brief Blits a sprite at the given screen position, skipping transparent pixels.
 * @param sprite Pointer to the source sprite.
 * @param x      Destination x coordinate.
 * @param y      Destination y coordinate.
 * @return 0 on success, 1 if @p sprite is @c NULL.
 */
int drawSprite(Sprite *sprite, int x, int y) {
  uint16_t row, col;

  if (sprite == NULL || sprite->colors == NULL) return 1;

  for (row = 0; row < sprite->height; row++) {
    for (col = 0; col < sprite->width; col++) {
      uint32_t color = sprite_rgb(sprite->colors[row * sprite->width + col]);
      if (color == TRANSPARENT_RGB) continue;
      if (draw_clipped_pixel(x + col, y + row, color) != 0) return 1;
    }
  }

  return 0;
}

/**
 * @brief Blits a sprite scaled to the given dimensions using nearest-neighbour sampling.
 * @param sprite Pointer to the source sprite.
 * @param x      Destination x coordinate.
 * @param y      Destination y coordinate.
 * @param width  Destination width in pixels.
 * @param height Destination height in pixels.
 * @return 0 on success, 1 if @p sprite is @c NULL or dimensions are invalid.
 */
int drawSpriteScaled(Sprite *sprite, int x, int y, int width, int height) {
  int row, col;

  if (sprite == NULL || sprite->colors == NULL || width <= 0 || height <= 0) return 1;

  for (row = 0; row < height; row++) {
    int src_y = row * sprite->height / height;
    for (col = 0; col < width; col++) {
      int src_x = col * sprite->width / width;
      uint32_t color = sprite_rgb(sprite->colors[src_y * sprite->width + src_x]);

      if (color == TRANSPARENT_RGB) continue;
      if (draw_clipped_pixel(x + col, y + row, color) != 0) return 1;
    }
  }

  return 0;
}

/**
 * @brief Draws a colourised button by substituting template pixel colours:
 *        fill regions → @p color (shaded to 75% if selected), highlight → 125% shade,
 *        border → near-black.  Falls back to returning 1 if sprites are unavailable.
 * @param x        Destination x coordinate.
 * @param y        Destination y coordinate.
 * @param width    Button width in pixels.
 * @param height   Button height in pixels.
 * @param selected @c true uses the pressed-button template.
 * @param color    Base fill colour.
 * @return 0 on success, 1 if the sprite data is unavailable.
 */
int drawButtonSprite(int x, int y, int width, int height, bool selected, uint32_t color) {
  Sprite *sprite = selected ? buttonPressedSprite : buttonSprite;
  uint32_t fill = selected ? shade_color(color, 75) : color;
  int row, col;

  if (sprite == NULL || sprite->colors == NULL || width <= 0 || height <= 0) return 1;

  for (row = 0; row < height; row++) {
    int src_y = row * sprite->height / height;
    for (col = 0; col < width; col++) {
      int src_x = col * sprite->width / width;
      uint32_t px = sprite_rgb(sprite->colors[src_y * sprite->width + src_x]);
      uint32_t out = px;

      if (px == TRANSPARENT_RGB) continue;
      if (px == TEMPLATE_FILL || px == TEMPLATE_PRESSED_FILL) out = fill;
      else if (px == TEMPLATE_HIGHLIGHT) out = shade_color(color, 125);
      else if (px == TEMPLATE_BORDER) out = selected ? rgb(35, 35, 35) : rgb(0, 0, 0);

      if (draw_clipped_pixel(x + col, y + row, out) != 0) return 1;
    }
  }

  return 0;
}

/**
 * @brief Draws the mouse cursor sprite at the given position.
 * @param x Cursor x coordinate.
 * @param y Cursor y coordinate.
 * @return 0 on success, 1 if the sprite is not loaded.
 */
int drawMouseCursorSprite(int x, int y) {
  return drawSprite(mouseCursorSprite, x, y);
}

/**
 * @brief Draws the order ticket sprite scaled to the given dimensions.
 * @param x      Destination x coordinate.
 * @param y      Destination y coordinate.
 * @param width  Destination width.
 * @param height Destination height.
 * @return 0 on success, 1 if the sprite is not loaded.
 */
int drawOrderTicketSprite(int x, int y, int width, int height) {
  return drawSpriteScaled(orderTicketSprite, x, y, width, height);
}

/**
 * @brief Draws the "open" sign sprite scaled to the given dimensions.
 * @param x      Destination x coordinate.
 * @param y      Destination y coordinate.
 * @param width  Destination width.
 * @param height Destination height.
 * @return 0 on success, 1 if the sprite is not loaded.
 */
int drawOpenSignSprite(int x, int y, int width, int height) {
  return drawSpriteScaled(openSignSprite, x, y, width, height);
}

/**
 * @brief Draws the "closed" sign sprite scaled to the given dimensions.
 * @param x      Destination x coordinate.
 * @param y      Destination y coordinate.
 * @param width  Destination width.
 * @param height Destination height.
 * @return 0 on success, 1 if the sprite is not loaded.
 */
int drawClosedSignSprite(int x, int y, int width, int height) {
  return drawSpriteScaled(closedSignSprite, x, y, width, height);
}

/**
 * @brief Loads all required sprites from their embedded XPM sources.
 *        The four core sprites (button, button-pressed, cursor, ticket) must succeed;
 *        the sign sprites are loaded on a best-effort basis.
 * @return 0 on success, 1 if any core sprite fails to load.
 */
int loadSprites(void) {
  buttonSprite = createSprite((xpm_map_t) button_xpm);
  buttonPressedSprite = createSprite((xpm_map_t) button_pressed_xpm);
  mouseCursorSprite = createSprite((xpm_map_t) mouse_cursor_xpm);
  orderTicketSprite = createSprite((xpm_map_t) order_ticket_xpm);

  if (buttonSprite == NULL || buttonPressedSprite == NULL ||
      mouseCursorSprite == NULL || orderTicketSprite == NULL) {
    unloadSprites();
    return 1;
  }

  openSignSprite   = createSprite((xpm_map_t) a81255e6075e4521b28c45608627c6b0MtbZSeLNMhCw9l3n);
  closedSignSprite = createSprite((xpm_map_t) b9eb1cb3924a4a89cd449e69fb35d556fHKr1debNstGhuGO);

  return 0;
}

/**
 * @brief Destroys all loaded sprites and resets every global sprite pointer to @c NULL.
 */
void unloadSprites(void) {
  destroy_sprite(buttonSprite);
  destroy_sprite(buttonPressedSprite);
  destroy_sprite(mouseCursorSprite);
  destroy_sprite(orderTicketSprite);
  destroy_sprite(openSignSprite);
  destroy_sprite(closedSignSprite);

  buttonSprite        = NULL;
  buttonPressedSprite = NULL;
  mouseCursorSprite   = NULL;
  orderTicketSprite   = NULL;
  openSignSprite      = NULL;
  closedSignSprite    = NULL;
}

#pragma clang optimize on
