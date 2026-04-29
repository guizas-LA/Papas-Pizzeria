#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t *pixmap;
  xpm_image_t img;
  uint16_t x;
  uint16_t y;
} sprite_t;

typedef struct {
  uint8_t no_pixmaps;
  uint8_t cur_pixmap;
  sprite_t pixmaps[8];
  uint16_t x;
  uint16_t y;
} animated_sprite_t;

int(set_graphics_mode)(uint16_t mode);
int(map_video_memory)(uint16_t mode);
int(vg_clear_buffer)(uint32_t color);
int(vg_swap_buffer)();
int(vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color);
int(vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int(vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
int(vg_draw_xpm)(uint8_t *pixmap, xpm_image_t img, uint16_t x, uint16_t y);
int(vg_draw_sprite)(const sprite_t *sprite);
int(animated_sprite_next_frame)(animated_sprite_t *sprite);
int(animated_sprite_draw)(const animated_sprite_t *sprite);
bool(sprites_collide)(const sprite_t *a, const sprite_t *b);

#endif
