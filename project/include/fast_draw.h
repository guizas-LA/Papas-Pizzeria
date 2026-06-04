#ifndef PROJECT_FAST_DRAW_H
#define PROJECT_FAST_DRAW_H

#include <stdint.h>
#include <lcom/lcf.h>

int fast_draw_init(uint16_t mode);

void fast_clear(uint32_t color);

void fast_rect(int x, int y, int w, int h, uint32_t color);

void fast_circle(int cx, int cy, int radius, uint32_t color);

void fast_pixel(int x, int y, uint32_t color);

void fast_xpm(uint8_t *pixmap, xpm_image_t img, int x, int y);

void fast_swap(void);

#endif
