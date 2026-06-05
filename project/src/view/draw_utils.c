#include "draw_utils.h"
#include "sprites.h"

#include <string.h>
#include <stdlib.h>

#pragma clang optimize off

#define SCREEN_W 800
#define SCREEN_H 600

static uint8_t *fb_buf  = NULL;
static uint8_t *vram    = NULL;
static unsigned bpp     = 3;
static unsigned fb_size = 0;


int draw_init(uint16_t mode) {
  vbe_mode_info_t vmi;
  struct minix_mem_range mr;
  unsigned int vram_size;

  if (vbe_get_mode_info(mode, &vmi) != OK) return 1;

  bpp       = (vmi.BitsPerPixel + 7) / 8;
  vram_size = vmi.XResolution * vmi.YResolution * bpp;
  fb_size   = vram_size;

  mr.mr_base  = (phys_bytes) vmi.PhysBasePtr;
  mr.mr_limit = mr.mr_base + vram_size;
  sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr);

  vram = vm_map_phys(SELF, (void *) mr.mr_base, vram_size);
  if (vram == MAP_FAILED) { vram = NULL; return 1; }

  fb_buf = malloc(vram_size);
  if (fb_buf == NULL) return 1;

  memset(fb_buf, 0, fb_size);
  return 0;
}

void draw_swap(void) {
  if (vram != NULL && fb_buf != NULL)
    memcpy(vram, fb_buf, fb_size);
}

void draw_clear(uint32_t color) {
  unsigned int x;
  uint8_t pixel[4];
  if (fb_buf == NULL) return;

  memcpy(pixel, &color, bpp);

  for (x = 0; x < SCREEN_W; x++)
    memcpy(&fb_buf[x * bpp], pixel, bpp);

  {
    unsigned int row_bytes = SCREEN_W * bpp;
    unsigned int y;
    for (y = 1; y < SCREEN_H; y++)
      memcpy(&fb_buf[y * row_bytes], fb_buf, row_bytes);
  }
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
  int x0, y0, x1, y1, row, col;
  uint8_t pixel[4];
  unsigned int row_bytes;

  if (fb_buf == NULL || w <= 0 || h <= 0) return;

  x0 = x < 0 ? 0 : x;
  y0 = y < 0 ? 0 : y;
  x1 = (x + w > SCREEN_W) ? SCREEN_W : x + w;
  y1 = (y + h > SCREEN_H) ? SCREEN_H : y + h;
  if (x0 >= x1 || y0 >= y1) return;

  memcpy(pixel, &color, bpp);
  row_bytes = SCREEN_W * bpp;

  for (col = x0; col < x1; col++)
    memcpy(&fb_buf[y0 * row_bytes + col * bpp], pixel, bpp);

  {
    unsigned int src_off = y0 * row_bytes + x0 * bpp;
    unsigned int span    = (unsigned int)(x1 - x0) * bpp;
    for (row = y0 + 1; row < y1; row++)
      memcpy(&fb_buf[row * row_bytes + x0 * bpp], &fb_buf[src_off], span);
  }
}

void draw_circle(int cx, int cy, int radius, uint32_t color) {
  int y, half;
  int r2 = radius * radius;
  uint8_t pixel[4];
  unsigned int row_bytes;
  int x0, x1, py, col;

  if (fb_buf == NULL || radius <= 0) return;
  memcpy(pixel, &color, bpp);
  row_bytes = SCREEN_W * bpp;

  for (y = -radius; y <= radius; y++) {
    half = 0;
    while ((half + 1) * (half + 1) <= r2 - y * y) half++;

    py = cy + y;
    if (py < 0 || py >= SCREEN_H) continue;

    x0 = cx - half;
    x1 = cx + half + 1;
    if (x0 < 0) x0 = 0;
    if (x1 > SCREEN_W) x1 = SCREEN_W;
    if (x0 >= x1) continue;

    for (col = x0; col < x1; col++)
      memcpy(&fb_buf[py * row_bytes + col * bpp], pixel, bpp);
  }
}

void draw_pixel(int x, int y, uint32_t color) {
  if (fb_buf == NULL) return;
  if (x < 0 || y < 0 || x >= SCREEN_W || y >= SCREEN_H) return;
  memcpy(&fb_buf[(y * SCREEN_W + x) * bpp], &color, bpp);
}

void draw_xpm(uint8_t *pixmap, xpm_image_t img, int x, int y) {
  unsigned int row;
  unsigned int row_bytes, src_row_bytes;

  if (fb_buf == NULL || pixmap == NULL) return;

  row_bytes     = SCREEN_W * bpp;
  src_row_bytes = img.width * bpp;

  for (row = 0; row < img.height; row++) {
    int dy = y + (int)row;
    int sx = 0, dx = x;
    unsigned int copy_w = img.width;

    if (dy < 0 || dy >= SCREEN_H) continue;
    if (dx < 0) { sx = -dx; copy_w -= (unsigned int)(-dx); dx = 0; }
    if (dx + (int)copy_w > SCREEN_W) copy_w = (unsigned int)(SCREEN_W - dx);
    if (copy_w == 0) continue;

    memcpy(&fb_buf[dy * row_bytes + dx * bpp],
           &pixmap[row * src_row_bytes + sx * bpp],
           copy_w * bpp);
  }
}

static const uint8_t FONT_DATA[][7] = {
  /* A:0  */ {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
  /* B:1  */ {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E},
  /* C:2  */ {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E},
  /* D:3  */ {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E},
  /* E:4  */ {0x1F, 0x10, 0x10, 0x1C, 0x10, 0x10, 0x1F},
  /* F:5  */ {0x1F, 0x10, 0x10, 0x1C, 0x10, 0x10, 0x10},
  /* G:6  */ {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E},
  /* H:7  */ {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
  /* I:8  */ {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F},
  /* J:9  */ {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C},
  /* K:10 */ {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},
  /* L:11 */ {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F},
  /* M:12 */ {0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11},
  /* N:13 */ {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11},
  /* O:14 */ {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
  /* P:15 */ {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},
  /* Q:16 */ {0x0E, 0x11, 0x11, 0x11, 0x13, 0x11, 0x0E},
  /* R:17 */ {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11},
  /* S:18 */ {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E},
  /* T:19 */ {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
  /* U:20 */ {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
  /* V:21 */ {0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04, 0x04},
  /* X:22 */ {0x11, 0x0A, 0x04, 0x04, 0x04, 0x0A, 0x11},
  /* Y:23 */ {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04},
  /* Z:24 */ {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F},
  /* 0:25 */ {0x0E, 0x13, 0x15, 0x15, 0x19, 0x11, 0x0E},
  /* 1:26 */ {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
  /* 2:27 */ {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F},
  /* 3:28 */ {0x0E, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0E},
  /* 4:29 */ {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},
  /* 5:30 */ {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
  /* 6:31 */ {0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E},
  /* 7:32 */ {0x1F, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04},
  /* 8:33 */ {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},
  /* 9:34 */ {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E},
  /* ?:35 */ {0x0E, 0x11, 0x01, 0x06, 0x04, 0x00, 0x04},
  /* W:36 */ {0x11, 0x11, 0x11, 0x15, 0x1B, 0x1B, 0x0A},
  /* ::37 */ {0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00},
  /* Á:38 */ {0x02, 0x0E, 0x11, 0x1F, 0x11, 0x11, 0x11},
  /* À:39 */ {0x08, 0x0E, 0x11, 0x1F, 0x11, 0x11, 0x11},
  /* Ç:40 */ {0x0E, 0x11, 0x10, 0x10, 0x11, 0x0E, 0x06},
  /* Ã:41 */ {0x0A, 0x0E, 0x11, 0x1F, 0x11, 0x11, 0x11},
  /* Õ:42 */ {0x0A, 0x0E, 0x11, 0x11, 0x11, 0x11, 0x0E},
  /* +:43 */ {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00},
};

static int char_to_font_idx(char c) {
  switch (c) {
    case 'A': return 0;  case 'B': return 1;  case 'C': return 2;
    case 'D': return 3;  case 'E': return 4;  case 'F': return 5;
    case 'G': return 6;  case 'H': return 7;  case 'I': return 8;
    case 'J': return 9;  case 'K': return 10; case 'L': return 11;
    case 'M': return 12; case 'N': return 13; case 'O': return 14;
    case 'P': return 15; case 'Q': return 16; case 'R': return 17;
    case 'S': return 18; case 'T': return 19; case 'U': return 20;
    case 'V': return 21; case 'W': return 36; case 'X': return 22;
    case 'Y': return 23; case 'Z': return 24;
    case '0': return 25; case '1': return 26; case '2': return 27;
    case '3': return 28; case '4': return 29; case '5': return 30;
    case '6': return 31; case '7': return 32; case '8': return 33;
    case '9': return 34;
    case ':': return 37;
    case '+': return 43;
    default:  return 35;
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
        draw_rect(x + col * scale, y + row * scale, scale, scale, color);
      }
    }
  }
}

void draw_string(int x, int y, const char *s, int scale, uint32_t color) {
  int char_step = 6 * scale;
  int i = 0;
  int pos = 0;
  while (s[i] != '\0') {
    unsigned char c = (unsigned char)s[i];
    int z;
    if (c == ' ') { i++; pos += char_step; continue; }
    if (c == 0xC3 && s[i + 1] != '\0') {
      unsigned char n = (unsigned char)s[i + 1];
      if      (n == 0x81) z = 38;
      else if (n == 0x80) z = 39;
      else if (n == 0x87) z = 40;
      else if (n == 0x83) z = 41;
      else if (n == 0x95) z = 42;
      else z = 35;
      i += 2;
    } else {
      z = char_to_font_idx((char)c);
      i++;
    }
    {
      const uint8_t *rows = FONT_DATA[z];
      int row, col;
      for (row = 0; row < 7; row++)
        for (col = 0; col < 5; col++)
          if (rows[row] & (1 << (4 - col)))
            draw_rect(x + pos + col * scale, y + row * scale, scale, scale, color);
    }
    pos += char_step;
  }
}

void draw_panel(int x, int y, int w, int h, uint32_t color) {
  draw_rect(x, y, w, h, color);
  draw_rect(x + 4, y + 4, w - 8, h - 8, rgb(252, 238, 202));
}

void draw_button(int x, int y, int w, int h, bool selected, uint32_t color) {
  if (drawButtonSprite(x, y, w, h, selected, color) != 0) {
    draw_rect(x, y, w, h, selected ? rgb(48, 120, 70) : rgb(80, 80, 80));
    draw_rect(x + 4, y + 4, w - 8, h - 8, color);
  }
}

void draw_xpm_scaled(uint8_t *pixmap, xpm_image_t img, int dst_w, int dst_h) {
  int dst_row, dst_col, src_row, src_col;
  unsigned int si, di;
  if (fb_buf == NULL || pixmap == NULL || dst_w <= 0 || dst_h <= 0) return;
  for (dst_row = 0; dst_row < dst_h; dst_row++) {
    if (dst_row >= SCREEN_H) break;
    src_row = dst_row * (int)img.height / dst_h;
    for (dst_col = 0; dst_col < dst_w; dst_col++) {
      if (dst_col >= SCREEN_W) break;
      src_col = dst_col * (int)img.width / dst_w;
      si = (unsigned int)(src_row * (int)img.width + src_col) * bpp;
      di = (unsigned int)(dst_row * SCREEN_W + dst_col) * bpp;
      memcpy(&fb_buf[di], &pixmap[si], bpp);
    }
  }
}

#pragma clang optimize on
