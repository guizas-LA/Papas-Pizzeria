// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include "graphics.h"
#include "utils.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
    // sets the language of LCF messages (can be either EN-US or PT-PT)
    lcf_set_language("EN-US");

    // enables to log function invocations that are being "wrapped" by LCF
    // [comment this out if you don't want/need it]
    lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

    // enables to save the output of printf function calls on a file
    // [comment this out if you don't want/need it]
    lcf_log_output("/home/lcom/labs/lab5/output.txt");

    // handles control over to LCF
    // [LCF handles command line arguments and invokes the right function]
    if (lcf_start(argc, argv))
        return 1;

    // LCF clean up tasks
    // [must be the last statement before return]
    lcf_cleanup();

    return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {
    if (set_graphics_mode(mode) != 0) {
        printf("%s: failed to set graphics mode\n", __func__);
        return 1;
    }

    sleep(delay);

    if (vg_exit() != OK) {
        printf("%s: vg_exit() failed\n", __func__);
        return 1;
    }

    return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
    if (map_video_memory(mode) != 0) {
        printf("%s: failed to map video memory\n", __func__);
        return 1;
    }

    if (set_graphics_mode(mode) != 0) {
        printf("%s: failed to set graphics mode\n", __func__);
        return 1;
    }

    if (vg_clear_buffer(0) != 0) {
        vg_exit();
        return 1;
    }

    if (vg_draw_rectangle(x, y, width, height, color) != 0) {
        vg_exit();
        return 1;
    }

    if (vg_swap_buffer() != 0) {
        vg_exit();
        return 1;
    }

    if (wait_for_esc_breakcode() != 0) {
        vg_exit();
        return 1;
    }

    if (vg_exit() != OK) {
        return 1;
    }

    return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
    xpm_image_t img;
    uint8_t *pixmap;

    if (map_video_memory(0x105) != 0) {
        printf("%s: failed to map video memory\n", __func__);
        return 1;
    }

    if (set_graphics_mode(0x105) != 0) {
        printf("%s: failed to set graphics mode\n", __func__);
        return 1;
    }

    if (vg_clear_buffer(0) != 0) {
        vg_exit();
        return 1;
    }

    pixmap = xpm_load(xpm, XPM_INDEXED, &img);
    if (pixmap == NULL) {
        vg_exit();
        return 1;
    }

    if (vg_draw_xpm(pixmap, img, x, y) != 0) {
        free(pixmap);
        vg_exit();
        return 1;
    }

    if (vg_swap_buffer() != 0) {
        free(pixmap);
        vg_exit();
        return 1;
    }

    free(pixmap);

    if (wait_for_esc_breakcode() != 0) {
        vg_exit();
        return 1;
    }

    if (vg_exit() != OK) {
        return 1;
    }

    return 0;
}
