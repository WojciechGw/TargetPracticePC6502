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
    int8_t nb_shots;
    int8_t target_status;
    int8_t space_key_down;
} game_t;

game_t game;

#define WIDTH 320
#define HEIGHT 240
#define TARGET 0
#define TARGET_BOOM 1
#define CROSSHAIR 2

static const mode3_config_t background_config = {
    false, false, 0, 0, 320, 240,
    XRAM_BACKGROUND, XRAM_BACKGROUND_PALETTE};

static const mode3_config_t game_over_config = {
    false, false, 96, 80, 128, 75,
    XRAM_BACKGROUND, XRAM_BACKGROUND_PALETTE};

static mode5_csprite_t elts_sprite[3] = {
    {10, 150, XRAM_TARGET, XRAM_TARGET_PALETTE, MODE5_SIZE(16, 16), MODE5_4BPP},
    {10, 150, XRAM_TARGET_BOOM, XRAM_TARGET_PALETTE, MODE5_SIZE(16, 16), MODE5_1BPP},
    {100, 10, XRAM_CROSSHAIR, XRAM_CROSSHAIR_PALETTE, MODE5_SIZE(32, 32), MODE5_2BPP}
};

static mode5_csprite_t crosshair_sprite = {
    100, 10, XRAM_CROSSHAIR, XRAM_CROSSHAIR_PALETTE,
    MODE5_SIZE(32, 32), MODE5_2BPP};

static mode5_csprite_t target_sprite = {
    10, 150, XRAM_TARGET, XRAM_TARGET_PALETTE,
    MODE5_SIZE(16, 16), MODE5_4BPP};

static mode5_csprite_t target_boom_sprite = {
    10, 150, XRAM_TARGET_BOOM, XRAM_TARGET_PALETTE,
    MODE5_SIZE(16, 16), MODE5_1BPP};

static mode5_csprite_t shells_sprite[10] = {
    { 8, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 16, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 24, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 32, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 40, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 48, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 56, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 64, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 72, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 80, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP }
};

uint8_t pad[4];

unsigned char dirs, jump, fire;

void reset_target();

void init_game() {
    game.space_key_down = 0;
    game.target_dx = 4;
    game.target_dy = -20;
    game.nb_shots = 10;
    reset_target();
}

void read_player1(void)
{
    unsigned char dpad = xram0_peek8(0xFF00);
    unsigned char sticks = xram0_peek8(0xFF01);
    unsigned char btn0 = xram0_peek8(0xFF02);
    dirs = (dpad | sticks) & 0x0F; /* up 1, down 2, left 4, right 8 */
    jump = btn0 & 0x09;            /* A or X */
    fire = btn0 & 0x12;            /* B or Y */
}

void reset_target() {
    elts_sprite[TARGET].x_pos_px = 0;
    elts_sprite[TARGET].y_pos_px = 150;
    elts_sprite[TARGET_BOOM].x_pos_px = 0;
    elts_sprite[TARGET_BOOM].y_pos_px = 250;
    game.target_dx = 4;
    game.target_dy = -30;
    game.target_status = 0;
}

void move_target() {
    if (game.target_status == 0) {
        elts_sprite[TARGET].x_pos_px += game.target_dx;
        elts_sprite[TARGET].y_pos_px += (game.target_dy >> 4);
    } else {
        elts_sprite[TARGET_BOOM].x_pos_px += game.target_dx;
        elts_sprite[TARGET_BOOM].y_pos_px += (game.target_dy >> 4);
    }
    game.target_dy++;

    if (elts_sprite[TARGET].x_pos_px >= 316) {
        reset_target();
    }
}

void shoot() {
    if (game.nb_shots == 0) {
        return;
    }
    game.nb_shots--;
    shells_sprite[game.nb_shots].y_pos_px = 250;
    // Check if the target is hit
    if (elts_sprite[CROSSHAIR].x_pos_px > elts_sprite[TARGET].x_pos_px+10) return;
    if (elts_sprite[CROSSHAIR].x_pos_px < elts_sprite[TARGET].x_pos_px-24) return;
    if (elts_sprite[CROSSHAIR].y_pos_px > elts_sprite[TARGET].y_pos_px+6) return;
    if (elts_sprite[CROSSHAIR].y_pos_px < elts_sprite[TARGET].y_pos_px-22) return;

    // Hit the target
    game.target_status = 32;
    elts_sprite[TARGET_BOOM].x_pos_px = elts_sprite[TARGET].x_pos_px;
    elts_sprite[TARGET_BOOM].y_pos_px = elts_sprite[TARGET].y_pos_px;
    target_sprite.options = MODE5_1BPP;

    elts_sprite[TARGET].y_pos_px = 250;
    xram0_write(XRAM_TARGET_SPRITE, &target_boom_sprite, sizeof(target_boom_sprite));
}

static keyboard_t k;

void handle_input() {
        xram0_read(&k, XRAM_KEYBOARD, sizeof(k));
        if (KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_LEFT)) {
            if (crosshair_sprite.x_pos_px) {
                    elts_sprite[CROSSHAIR].x_pos_px-=2;
                }
        } else if (KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_RIGHT)) {
            if (crosshair_sprite.x_pos_px < 288) {
                    elts_sprite[CROSSHAIR].x_pos_px+=2;
                }
        }

        if (KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_UP)) {
            if (crosshair_sprite.y_pos_px) {
                    elts_sprite[CROSSHAIR].y_pos_px-=2;
                }
        } else if (KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_DOWN)) {
            if (crosshair_sprite.y_pos_px < 224) {
                    elts_sprite[CROSSHAIR].y_pos_px+=2;
                }
        }

        if (KEYBOARD_PRESSED(k.keys, HID_KEY_SPACE)) {
            if (!game.space_key_down) {
                shoot();
                game.space_key_down = 1;
            }
        } else {
            game.space_key_down = 0;
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
//    xreg_vga_mode5(MODE5_CUSTOM, XRAM_TARGET_SPRITE, 1, 0, 0, 0);
//    xreg_vga_mode5(MODE5_CUSTOM, XRAM_TARGET_BOOM_SPRITE, 1, 1, 0, 0);
//    xreg_vga_mode5(MODE5_CUSTOM, XRAM_CROSSHAIR_SPRITE, 1, 1, 0, 0);
    xreg_vga_mode5(MODE5_CUSTOM, XRAM_ELTS_SPRITE, 3, 1, 0, 0);
    xreg_vga_mode5(MODE5_CUSTOM, XRAM_SHELL_SPRITE, 10, 2, 0, 0);
    // TODO: learn how to add a second bitmap over the first (if it's possible)
//    xreg_vga_mode3(MODE3_4BPP, XRAM_GAME_OVER_CONFIG);

    init_game();
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
        xram0_write(XRAM_ELTS_SPRITE, &elts_sprite, sizeof(elts_sprite));
        if (game.target_status) {
            game.target_status--;
            if (!game.target_status) {
                reset_target();
            }
        }

        xram0_write(XRAM_CROSSHAIR_SPRITE, &crosshair_sprite, sizeof(crosshair_sprite));
        xram0_write(XRAM_SHELL_SPRITE, &shells_sprite, sizeof(shells_sprite));
    };
}
