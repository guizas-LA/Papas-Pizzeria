#include "fast_draw.h"
#include <string.h>
#include <stdlib.h>

#define SCREEN_W 800
#define SCREEN_H 600

static uint8_t *fb_buf  = NULL;
static uint8_t *vram    = NULL;
static unsigned bpp     = 3;
static unsigned fb_size = 0;

int fast_draw_init(uint16_t mode) {
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

void fast_clear(uint32_t color) {
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

void fast_rect(int x, int y, int w, int h, uint32_t color) {
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

void fast_circle(int cx, int cy, int radius, uint32_t color) {
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

void fast_pixel(int x, int y, uint32_t color) {
  if (fb_buf == NULL) return;
  if (x < 0 || y < 0 || x >= SCREEN_W || y >= SCREEN_H) return;
  memcpy(&fb_buf[(y * SCREEN_W + x) * bpp], &color, bpp);
}

void fast_xpm(uint8_t *pixmap, xpm_image_t img, int x, int y) {
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

void fast_swap(void) {
  if (vram != NULL && fb_buf != NULL)
    memcpy(vram, fb_buf, fb_size);
}
