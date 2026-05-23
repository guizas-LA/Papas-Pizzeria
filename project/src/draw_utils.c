#include "draw_utils.h"
#include "graphics.h"

static const uint8_t FONT_DATA[][7] = {
  /* A */ {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
  /* D */ {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E},
  /* E */ {0x1F, 0x10, 0x10, 0x1C, 0x10, 0x10, 0x1F},
  /* I */ {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F},
  /* L */ {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F},
  /* M */ {0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11},
  /* N */ {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11},
  /* O */ {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
  /* P */ {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},
  /* R */ {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11},
  /* S */ {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E},
  /* T */ {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
  /* U */ {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
  /* Y */ {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04},
  /* Z */ {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F},
  /* B */ {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E},
  /* C */ {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E},
  /* G */ {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E},
  /* V */ {0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04, 0x04},
  /* X */ {0x11, 0x0A, 0x04, 0x04, 0x04, 0x0A, 0x11},
  /* 0 */ {0x0E, 0x13, 0x15, 0x15, 0x19, 0x11, 0x0E},
  /* 1 */ {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
  /* 2 */ {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F},
  /* 3 */ {0x0E, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0E},
  /* 4 */ {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},
  /* 5 */ {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
  /* 6 */ {0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E},
  /* 7 */ {0x1F, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04},
  /* 8 */ {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},
  /* 9 */ {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E},
  /* ? */ {0x0E, 0x11, 0x01, 0x06, 0x04, 0x00, 0x04},
};

static int char_to_font_idx(char c) {
  switch (c) {
    case 'A': return 0;  case 'D': return 1;  case 'E': return 2;
    case 'I': return 3;  case 'L': return 4;  case 'M': return 5;
    case 'N': return 6;  case 'O': return 7;  case 'P': return 8;
    case 'R': return 9;  case 'S': return 10; case 'T': return 11;
    case 'U': return 12; case 'Y': return 13; case 'Z': return 14;
    case 'B': return 15; case 'C': return 16; case 'G': return 17;
    case 'V': return 18; case 'X': return 19;
    case '0': return 20; case '1': return 21; case '2': return 22;
    case '3': return 23; case '4': return 24; case '5': return 25;
    case '6': return 26; case '7': return 27; case '8': return 28;
    case '9': return 29;
    default:  return 30;
  }
}

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (r << 16) | (g << 8) | b;
}

void draw_char(int x, int y, char c, int scale, uint32_t color) {
  const uint8_t *rows;
  int row, col;
  if (c == ' ') return;
  rows = FONT_DATA[char_to_font_idx(c)];
  for (row = 0; row < 7; row++) {
    for (col = 0; col < 5; col++) {
      if (rows[row] & (1 << (4 - col))) {
        vg_draw_rectangle(x + col * scale, y + row * scale, scale, scale, color);
      }
    }
  }
}

void draw_string(int x, int y, const char *s, int scale, uint32_t color) {
  int char_step = 6 * scale;
  int i;
  for (i = 0; s[i] != '\0'; i++) {
    draw_char(x + i * char_step, y, s[i], scale, color);
  }
}

void draw_circle(int cx, int cy, int radius, uint32_t color) {
  int y, x;
  for (y = -radius; y <= radius; y++) {
    for (x = -radius; x <= radius; x++) {
      if (x * x + y * y <= radius * radius) {
        vg_draw_pixel(cx + x, cy + y, color);
      }
    }
  }
}

void draw_panel(int x, int y, int w, int h, uint32_t color) {
  vg_draw_rectangle(x, y, w, h, color);
  vg_draw_rectangle(x + 4, y + 4, w - 8, h - 8, rgb(252, 238, 202));
}

void draw_button(int x, int y, int w, int h, bool selected, uint32_t color) {
  vg_draw_rectangle(x, y, w, h, selected ? rgb(48, 120, 70) : rgb(80, 80, 80));
  vg_draw_rectangle(x + 4, y + 4, w - 8, h - 8, color);
}

void draw_number(int x, int y, int value, uint32_t color) {
  int width = 0;
  int temp  = value;
  int i;

  if (temp == 0) width = 1;
  while (temp > 0) { width++; temp /= 10; }

  for (i = width - 1; i >= 0; i--) {
    int digit = value % 10;
    int px    = x + i * 22;

    if (digit != 1 && digit != 4)                              vg_draw_rectangle(px,      y,      16, 4,  color);
    if (digit != 1 && digit != 2 && digit != 3 && digit != 7) vg_draw_rectangle(px,      y,      4,  18, color);
    if (digit != 5 && digit != 6)                              vg_draw_rectangle(px + 12, y,      4,  18, color);
    if (digit != 0 && digit != 1 && digit != 7)               vg_draw_rectangle(px,      y + 17, 16, 4,  color);
    if (digit == 0 || digit == 2 || digit == 6 || digit == 8) vg_draw_rectangle(px,      y + 20, 4,  18, color);
    if (digit != 2)                                            vg_draw_rectangle(px + 12, y + 20, 4,  18, color);
    if (digit != 1 && digit != 4 && digit != 7)               vg_draw_rectangle(px,      y + 36, 16, 4,  color);

    value /= 10;
  }
}
