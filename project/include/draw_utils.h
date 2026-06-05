#ifndef PROJECT_DRAW_UTILS_H
#define PROJECT_DRAW_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <lcom/lcf.h>

int draw_init(uint16_t mode);

void draw_clear(uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_circle(int cx, int cy, int radius, uint32_t color);
void draw_pixel(int x, int y, uint32_t color);
void draw_xpm(uint8_t *pixmap, xpm_image_t img, int x, int y);
void draw_xpm_scaled(uint8_t *pixmap, xpm_image_t img, int dst_w, int dst_h);
void draw_swap(void);

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b);
void draw_char(int x, int y, char c, int scale, uint32_t color);
void draw_string(int x, int y, const char *s, int scale, uint32_t color);
void draw_panel(int x, int y, int w, int h, uint32_t color);
void draw_button(int x, int y, int w, int h, bool selected, uint32_t color);

#endif
