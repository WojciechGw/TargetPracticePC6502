/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include "xram.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <rp6502.h>

static uint8_t vsync;

typedef struct
{
    int8_t target_dx;
    int8_t target_dy;
    int8_t nb_targets;
    int8_t target_status;
} game_t;

#define WIDTH 320
#define HEIGHT 240

static const mode3_config_t background_config = {
    false, false, 0, 0, 320, 240,
    XRAM_BACKGROUND, XRAM_BACKGROUND_PALETTE};

static const mode3_config_t game_over_config = {
    false, false, 96, 80, 128, 75,
    XRAM_BACKGROUND, XRAM_BACKGROUND_PALETTE};

static mode5_csprite_t crosshair_sprite = {
    100, 10, XRAM_CROSSHAIR, XRAM_CROSSHAIR_PALETTE,
    MODE5_SIZE(32, 32), MODE5_2BPP};

static mode5_csprite_t target_sprite = {
    10, 150, XRAM_TARGET, XRAM_TARGET_PALETTE,
    MODE5_SIZE(16, 16), MODE5_4BPP};

static mode5_csprite_t target_boom_sprite = {
    10, 150, XRAM_TARGET, XRAM_TARGET_PALETTE,
    MODE5_SIZE(16, 16), MODE5_1BPP};

uint8_t pad[4];

unsigned char dirs, jump, fire;

void read_player1(void)
{
    unsigned char dpad = xram0_peek8(0xFF00);
    unsigned char sticks = xram0_peek8(0xFF01);
    unsigned char btn0 = xram0_peek8(0xFF02);
    dirs = (dpad | sticks) & 0x0F; /* up 1, down 2, left 4, right 8 */
    jump = btn0 & 0x09;            /* A or X */
    fire = btn0 & 0x12;            /* B or Y */
}

game_t game;

void reset_target() {
//    target_sprite.options = MODE5_4BPP;
    target_sprite.x_pos_px = 0;
    target_sprite.y_pos_px = 150;
    target_boom_sprite.x_pos_px = 0;
    target_boom_sprite.y_pos_px = 150;
    game.target_dx = 4;
    game.target_dy = -30;
    game.target_status = 0;
}

void move_target() {
//    if (game.target_status == 0) {
        target_sprite.x_pos_px += game.target_dx;
        target_sprite.y_pos_px += (game.target_dy >> 4);
//    } else {
//        target_boom_sprite.x_pos_px += game.target_dx;
//        target_boom_sprite.y_pos_px += (game.target_dy >> 4);
//    }
    game.target_dy++;

    if (target_sprite.x_pos_px >= 316) {
        reset_target();
    }
}

void shoot() {
    // Check if the target is hit
    if (crosshair_sprite.x_pos_px > target_sprite.x_pos_px+10) return;
    if (crosshair_sprite.x_pos_px < target_sprite.x_pos_px-24) return;
    if (crosshair_sprite.y_pos_px > target_sprite.y_pos_px+6) return;
    if (crosshair_sprite.y_pos_px < target_sprite.y_pos_px-22) return;

    // Hit the target
    game.target_status = 32;
//    target_boom_sprite.x_pos_px = target_sprite.x_pos_px;
//    target_boom_sprite.y_pos_px = target_sprite.y_pos_px;
    target_sprite.options = MODE5_1BPP;

    target_boom_sprite.y_pos_px = 250;
    xram0_write(XRAM_TARGET_SPRITE, &target_boom_sprite, sizeof(target_boom_sprite));
}

static keyboard_t k;

void handle_input() {
        xram0_read(&k, XRAM_KEYBOARD, sizeof(k));
        if (KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_LEFT)) {
            if (crosshair_sprite.x_pos_px) {
                    crosshair_sprite.x_pos_px-=2;
                }
        } else if (KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_RIGHT)) {
            if (crosshair_sprite.x_pos_px < 288) {
                    crosshair_sprite.x_pos_px+=2;
                }
        }

        if (KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_UP)) {
            if (crosshair_sprite.y_pos_px) {
                    crosshair_sprite.y_pos_px-=2;
                }
        } else if (KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_DOWN)) {
            if (crosshair_sprite.y_pos_px < 224) {
                    crosshair_sprite.y_pos_px+=2;
                }
        }

        if (KEYBOARD_PRESSED(k.keys, HID_KEY_SPACE)) {
            shoot();
        }
}

int main(void)
{
    xreg_ria_keyboard(XRAM_KEYBOARD);
    xreg_ria_gamepad(XRAM_GAMEPAD);

    // Use the 320x240 canvas
    xreg_vga_canvas(CANVAS_320X240);

    // Program the video mode
    xreg_vga_mode3(MODE3_4BPP, XRAM_BACKGROUND_CONFIG);
    xreg_vga_mode5(MODE5_4BPP | MODE5_16X16, XRAM_TARGET_SPRITE, 1, 0, 0, 0);
    xreg_vga_mode5(MODE5_1BPP | MODE5_16X16, XRAM_TARGET_BOOM_SPRITE, 1, 1, 0, 0);
    xreg_vga_mode5(MODE5_2BPP | MODE5_32X32, XRAM_CROSSHAIR_SPRITE, 1, 2, 0, 0);
    // TODO: learn how to add a second bitmap over the first (if it's possible)
//    xreg_vga_mode3(MODE3_4BPP, XRAM_GAME_OVER_CONFIG);

    game.target_dx = 4;
    game.target_dy = -20;
    vsync = ria_vsync();

    xram0_write(XRAM_BACKGROUND_CONFIG, &background_config, sizeof(background_config));
//    xram0_write(XRAM_GAME_OVER_CONFIG, &game_over_config, sizeof(game_over_config));

    while (true) {
        while (ria_vsync() == vsync)
            ;
        vsync = ria_vsync();
        move_target();

        handle_input();

        // Update the sprites
        if (game.target_status == 0) {
            xram0_write(XRAM_TARGET_SPRITE, &target_sprite, sizeof(target_sprite));
        } else {
            xram0_write(XRAM_TARGET_BOOM_SPRITE, &target_sprite, sizeof(target_sprite));
            game.target_status--;
            if (!game.target_status) {
                reset_target();
            }
        }

        xram0_write(XRAM_CROSSHAIR_SPRITE, &crosshair_sprite, sizeof(crosshair_sprite));
    };
}
