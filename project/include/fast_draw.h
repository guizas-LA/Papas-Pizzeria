#ifndef PROJECT_FAST_DRAW_H
#define PROJECT_FAST_DRAW_H

#include <stdint.h>
#include <lcom/lcf.h>

/**
 * Initialize fast drawing by getting a direct pointer to the back-buffer.
 * Must be called AFTER map_video_memory() + set_graphics_mode().
 * Returns 0 on success.
 */
int fast_draw_init(uint16_t mode);

/** Fast clear: fills entire buffer with a solid color */
void fast_clear(uint32_t color);

/** Fast filled rectangle: writes directly to buffer, with clipping */
void fast_rect(int x, int y, int w, int h, uint32_t color);

/** Fast filled circle using scanlines */
void fast_circle(int cx, int cy, int radius, uint32_t color);

/** Fast pixel write (no function-call overhead of vg_draw_pixel) */
void fast_pixel(int x, int y, uint32_t color);

/** Fast XPM blit: copies an XPM image using memcpy per row */
void fast_xpm(uint8_t *pixmap, xpm_image_t img, int x, int y);

/** Copy back-buffer to video memory (replaces vg_swap_buffer) */
void fast_swap(void);

#endif
