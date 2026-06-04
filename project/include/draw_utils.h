#ifndef PROJECT_DRAW_UTILS_H
#define PROJECT_DRAW_UTILS_H

#include <stdbool.h>
#include <stdint.h>

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b);
void draw_char(int x, int y, char c, int scale, uint32_t color);
void draw_string(int x, int y, const char *s, int scale, uint32_t color);
void draw_circle(int cx, int cy, int radius, uint32_t color);
void draw_panel(int x, int y, int w, int h, uint32_t color);
void draw_button(int x, int y, int w, int h, bool selected, uint32_t color);

#endif
