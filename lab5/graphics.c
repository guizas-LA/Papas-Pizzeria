#include "graphics.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char *video_mem;
static char *hidden_buffer;
static vbe_mode_info_t vmi;
static unsigned bytes_per_pixel;
static unsigned int frame_buffer_size;

int(set_graphics_mode)(uint16_t mode) {
  reg86_t reg86;
  memset(&reg86, 0, sizeof(reg86));

  reg86.intno = 0x10;
  reg86.ah = 0x4F;
  reg86.al = 0x02;
  reg86.bx = BIT(14) | mode;

  if (sys_int86(&reg86) != OK) {
    return 1;
  }

  return 0;
}

int(map_video_memory)(uint16_t mode) {
  struct minix_mem_range mr;
  unsigned int vram_base;
  unsigned int vram_size;
  int r;

  if (vbe_get_mode_info(mode, &vmi) != OK) {
    return 1;
  }

  vram_base = vmi.PhysBasePtr;
  vram_size = vmi.XResolution * vmi.YResolution * ((vmi.BitsPerPixel + 7) / 8);
  bytes_per_pixel = (vmi.BitsPerPixel + 7) / 8;

  mr.mr_base = (phys_bytes) vram_base;
  mr.mr_limit = mr.mr_base + vram_size;

  if ((r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)) != OK) {
    printf("sys_privctl (ADD_MEM) failed: %d\n", r);
    return 1;
  }

  video_mem = vm_map_phys(SELF, (void *) mr.mr_base, vram_size);

  if (video_mem == MAP_FAILED) {
    printf("couldn't map video memory\n");
    return 1;
  }

  if (hidden_buffer != NULL) {
    free(hidden_buffer);
  }

  hidden_buffer = malloc(vram_size);
  if (hidden_buffer == NULL) {
    printf("couldn't allocate hidden buffer\n");
    return 1;
  }

  frame_buffer_size = vram_size;
  memset(hidden_buffer, 0, frame_buffer_size);

  return 0;
}

int(vg_clear_buffer)(uint32_t color) {
  if (hidden_buffer == NULL) {
    return 1;
  }

  for (unsigned int offset = 0; offset < frame_buffer_size; offset += bytes_per_pixel) {
    memcpy(&hidden_buffer[offset], &color, bytes_per_pixel);
  }

  return 0;
}

int(vg_swap_buffer)() {
  if (video_mem == NULL || hidden_buffer == NULL) {
    return 1;
  }

  memcpy(video_mem, hidden_buffer, frame_buffer_size);
  return 0;
}

int(vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color) {
  if (hidden_buffer == NULL || x >= vmi.XResolution || y >= vmi.YResolution) {
    return 1;
  }

  unsigned int pixel_index = (y * vmi.XResolution + x) * bytes_per_pixel;
  memcpy(&hidden_buffer[pixel_index], &color, bytes_per_pixel);

  return 0;
}

int(vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
  for (uint16_t i = 0; i < len; i++) {
    if (vg_draw_pixel(x + i, y, color) != 0) {
      return 1;
    }
  }

  return 0;
}

int(vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  for (uint16_t i = 0; i < height; i++) {
    if (vg_draw_hline(x, y + i, width, color) != 0) {
      return 1;
    }
  }

  return 0;
}

int(vg_draw_xpm)(uint8_t *pixmap, xpm_image_t img, uint16_t x, uint16_t y) {
  if (pixmap == NULL || hidden_buffer == NULL) {
    return 1;
  }

  if (x + img.width > vmi.XResolution || y + img.height > vmi.YResolution) {
    return 1;
  }

  for (uint16_t row = 0; row < img.height; row++) {
    for (uint16_t col = 0; col < img.width; col++) {
      unsigned int src_index = (row * img.width + col) * bytes_per_pixel;
      unsigned int dst_index = ((y + row) * vmi.XResolution + (x + col)) * bytes_per_pixel;
      memcpy(&hidden_buffer[dst_index], &pixmap[src_index], bytes_per_pixel);
    }
  }

  return 0;
}

int(vg_draw_sprite)(const sprite_t *sprite) {
  if (sprite == NULL) {
    return 1;
  }

  return vg_draw_xpm(sprite->pixmap, sprite->img, sprite->x, sprite->y);
}

int(animated_sprite_next_frame)(animated_sprite_t *sprite) {
  if (sprite == NULL || sprite->no_pixmaps == 0) {
    return 1;
  }

  sprite->cur_pixmap = (sprite->cur_pixmap + 1) % sprite->no_pixmaps;
  return 0;
}

int(animated_sprite_draw)(const animated_sprite_t *sprite) {
  if (sprite == NULL || sprite->no_pixmaps == 0 || sprite->cur_pixmap >= sprite->no_pixmaps) {
    return 1;
  }

  sprite_t current = sprite->pixmaps[sprite->cur_pixmap];
  current.x = sprite->x;
  current.y = sprite->y;

  return vg_draw_sprite(&current);
}

bool(sprites_collide)(const sprite_t *a, const sprite_t *b) {
  if (a == NULL || b == NULL) {
    return false;
  }

  return a->x < b->x + b->img.width &&
         a->x + a->img.width > b->x &&
         a->y < b->y + b->img.height &&
         a->y + a->img.height > b->y;
}
