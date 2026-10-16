#include "xram.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <rp6502.h>

#define O_RDONLY 0x01
#define SPLASH_BITMAP 0
#define TARGET 0
#define TARGET_BOOM 1
#define HOSTAGE 2
#define BAD_GUY 3
#define CROSSHAIR 4
#define BACKGROUND_BITMAP 1
#define DIFFICULTY 5
#define HIDDEN 250

extern mode3_config_t background_config;
extern mode5_csprite_t elts_sprite[6];
extern mode5_csprite_t hud_sprite[27];
extern keyboard_t k;
extern gamepad_player_t pad;
extern uint8_t tmp;
extern uint8_t vsync;

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

uint8_t display_splash(uint8_t difficulty) {
    uint8_t pressed = 0;

    // Hide sprites used for the game
    elts_sprite[CROSSHAIR].y_pos_px = HIDDEN;
    elts_sprite[TARGET].y_pos_px = HIDDEN;
    elts_sprite[TARGET_BOOM].y_pos_px = HIDDEN;
    elts_sprite[HOSTAGE].x_pos_px = 320;
    elts_sprite[BAD_GUY].x_pos_px = 380;
    for (tmp=0; tmp<27; tmp++) {
        hud_sprite[tmp].y_pos_px = HIDDEN;
    }
    xram0_write(XRAM_ELTS_SPRITE, &elts_sprite, sizeof(elts_sprite));
    xram0_write(XRAM_HUD_SPRITE, &hud_sprite, sizeof(hud_sprite));

    load_file(SPLASH_BITMAP);
    elts_sprite[DIFFICULTY].y_pos_px = 100;
    if (difficulty == 0) {
        elts_sprite[DIFFICULTY].xram_sprite_ptr = XRAM_DIFF_EASY;
    } else if (difficulty == 1) {
        elts_sprite[DIFFICULTY].xram_sprite_ptr = XRAM_DIFF_MEDIUM;
    } else {
        elts_sprite[DIFFICULTY].xram_sprite_ptr = XRAM_DIFF_HARD;
    }

    xram0_read(&k, XRAM_KEYBOARD, sizeof(k));
    xram0_read(&pad, XRAM_GAMEPAD, sizeof(pad));
    tmp = pad.dpad & 0xF;

    while (!pad.btn0 && !KEYBOARD_PRESSED(k.keys, HID_KEY_SPACE)) {
        if ((tmp & 1) || KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_UP)) {
            if (!pressed) {
                pressed = 1;
                if (difficulty == 0) {
                    difficulty = 1;
                    elts_sprite[DIFFICULTY].xram_sprite_ptr = XRAM_DIFF_MEDIUM;
                } else if (difficulty == 1) {
                    difficulty = 2;
                    elts_sprite[DIFFICULTY].xram_sprite_ptr = XRAM_DIFF_HARD;
                } else {
                    difficulty = 0;
                    elts_sprite[DIFFICULTY].xram_sprite_ptr = XRAM_DIFF_EASY;
                }
            }
        } else if ((tmp & 2) || KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_DOWN)) {
            if (!pressed) {
                pressed = 1;
                if (difficulty == 0) {
                    difficulty = 2;
                    elts_sprite[DIFFICULTY].xram_sprite_ptr = XRAM_DIFF_HARD;
                } else if (difficulty == 1) {
                    difficulty = 0;
                    elts_sprite[DIFFICULTY].xram_sprite_ptr = XRAM_DIFF_EASY;
                } else {
                    difficulty = 1;
                    elts_sprite[DIFFICULTY].xram_sprite_ptr = XRAM_DIFF_MEDIUM;
                }
            }
        } else {
            pressed = 0;
        }

        xram0_write(XRAM_ELTS_SPRITE, &elts_sprite, sizeof(elts_sprite));
        while (ria_vsync() == vsync);
        vsync = ria_vsync();

        xram0_read(&k, XRAM_KEYBOARD, sizeof(k));
        xram0_read(&pad, XRAM_GAMEPAD, sizeof(pad));
        tmp = pad.dpad & 0xF;
    }        

    // Wait for button release
    while (1) {
        xram0_read(&k, XRAM_KEYBOARD, sizeof(k));
        xram0_read(&pad, XRAM_GAMEPAD, sizeof(pad));
        if (!pad.btn0 && !KEYBOARD_PRESSED(k.keys, HID_KEY_SPACE)) {
            break;
        }
    }

    load_file(BACKGROUND_BITMAP);
    elts_sprite[DIFFICULTY].y_pos_px = HIDDEN;

    return difficulty;
}
