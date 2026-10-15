#include "xram.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <rp6502.h>

#define O_RDONLY 0x01
#define SPLASH_BITMAP 0
#define BACKGROUND_BITMAP 1

extern mode3_config_t background_config;
extern keyboard_t k;
void wait_for_fire();

void load_file(uint8_t bitmap) {
    int fd;
    int n;

    if (bitmap == SPLASH_BITMAP) {
        background_config.xram_palette_ptr = XRAM_SPLASH_PALETTE;
        fd = open("ROM:splash.bin", O_RDONLY);
    } else {
        background_config.xram_palette_ptr = XRAM_BACKGROUND_PALETTE;
        fd = open("ROM:background.bin", O_RDONLY);
    }
    if (fd < 0) {
        return;
    }

    n = read_xram(XRAM_BACKGROUND, 0x7FFF, fd);
    n = read_xram(XRAM_BACKGROUND+0x7FFF, 5633, fd);
    xram0_write(XRAM_BACKGROUND_CONFIG, &background_config, sizeof(background_config));

    close(fd);
}

void display_splash() {
    load_file(SPLASH_BITMAP);

    wait_for_fire();

    load_file(BACKGROUND_BITMAP);
}
