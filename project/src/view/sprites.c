#include "sprites.h"

#include <stdlib.h>

#include "button_sprites.xpm"
#include "draw_utils.h"
#include "graphics.h"
#include "order_ticket.xpm"

#define TRANSPARENT_RGB 0x00FFFFFE
#define TEMPLATE_FILL 0x0000ED2F
#define TEMPLATE_PRESSED_FILL 0x00027A18
#define TEMPLATE_HIGHLIGHT 0x00FFFFFF
#define TEMPLATE_BORDER 0x00000000
#define SCREEN_W 800
#define SCREEN_H 600

Sprite *buttonSprite = NULL;
Sprite *buttonPressedSprite = NULL;
Sprite *mouseCursorSprite = NULL;
Sprite *orderTicketSprite = NULL;

static uint32_t sprite_rgb(uint32_t color) {
  return color & 0x00FFFFFF;
}

static int draw_clipped_pixel(int x, int y, uint32_t color) {
  if (x < 0 || y < 0 || x >= SCREEN_W || y >= SCREEN_H) return 0;
  return vg_draw_pixel((uint16_t) x, (uint16_t) y, color);
}

static uint32_t shade_color(uint32_t color, uint8_t percent) {
  unsigned r = ((color >> 16) & 0xFF) * percent / 100;
  unsigned g = ((color >> 8) & 0xFF) * percent / 100;
  unsigned b = (color & 0xFF) * percent / 100;

  if (r > 255) r = 255;
  if (g > 255) g = 255;
  if (b > 255) b = 255;

  return rgb(r, g, b);
}

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

void destroy_sprite(Sprite *sprite) {
  if (sprite == NULL) return;

  if (sprite->colors != NULL) {
    free(sprite->colors);
  }

  free(sprite);
}

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

int drawMouseCursorSprite(int x, int y) {
  return drawSprite(mouseCursorSprite, x, y);
}

int drawOrderTicketSprite(int x, int y, int width, int height) {
  Sprite *ticket = orderTicketSprite;
  return drawSpriteScaled(ticket, x, y, width, height);
}

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

  return 0;
}

void unloadSprites(void) {
  destroy_sprite(buttonSprite);
  destroy_sprite(buttonPressedSprite);
  destroy_sprite(mouseCursorSprite);
  destroy_sprite(orderTicketSprite);

  buttonSprite = NULL;
  buttonPressedSprite = NULL;
  mouseCursorSprite = NULL;
  orderTicketSprite = NULL;
}
