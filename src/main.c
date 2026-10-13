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
    int8_t nb_targets_hit;
    int8_t nb_shots;
    int8_t target_status;
    int8_t space_key_down;
    uint8_t hostage_step;
    uint8_t bad_guy_step;
    uint8_t easter_state;
    int8_t over;
} game_t;

game_t game;

#define WIDTH 320
#define HEIGHT 240
#define TARGET 0
#define TARGET_BOOM 1
#define HOSTAGE 2
#define BAD_GUY 3
#define CROSSHAIR 4
#define GAME_OVER1 20
#define GAME_OVER2 21
#define TARGET_HIT_H 22
#define TARGET_HIT_L 23
#define TARGET_LAUNCHED_H 25
#define TARGET_LAUNCHED_L 26

void game_over();

static const mode3_config_t background_config = {
    false, false, 0, 0, 320, 240,
    XRAM_BACKGROUND, XRAM_BACKGROUND_PALETTE};

static const mode3_config_t game_over_config = {
    false, false, 96, 80, 128, 75,
    XRAM_BACKGROUND, XRAM_BACKGROUND_PALETTE};

static mode5_csprite_t elts_sprite[5] = {
    {10, 150, XRAM_TARGET, XRAM_TARGET_PALETTE, MODE5_SIZE(16, 16), MODE5_4BPP},
    {10, 150, XRAM_TARGET_BOOM, XRAM_TARGET_PALETTE, MODE5_SIZE(16, 16), MODE5_1BPP},
    {320, 150, XRAM_HOSTAGE, XRAM_HOSTAGE_PALETTE, MODE5_SIZE(16, 32), MODE5_4BPP},
    {380, 150, XRAM_BAD_GUY, XRAM_BAD_GUY_PALETTE, MODE5_SIZE(32, 32), MODE5_4BPP},
    {100, 100, XRAM_CROSSHAIR, XRAM_CROSSHAIR_PALETTE, MODE5_SIZE(32, 32), MODE5_1BPP}
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

static mode5_csprite_t hud_sprite[27] = {
    // Shells
    { 8, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 16, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 24, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 32, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 40, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 48, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 56, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 64, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 72, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 80, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 88, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 96, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 104, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 112, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 120, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 128, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 136, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 144, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 152, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    { 160, 10, XRAM_SHELL, XRAM_SHELL_PALETTE, MODE5_SIZE(8, 16), MODE5_4BPP },
    // Game over
    { 96, 250, XRAM_GAME_OVER1, XRAM_GAME_OVER1_PALETTE, MODE5_SIZE(56, 64), MODE5_4BPP},
    { 152, 250, XRAM_GAME_OVER2, XRAM_GAME_OVER1_PALETTE, MODE5_SIZE(56, 64), MODE5_4BPP},
    // Score
    { 250, 10, XRAM_DIGITS, XRAM_DIGITS_PALETTE, MODE5_SIZE(8, 8), MODE5_1BPP},
    { 258, 10, XRAM_DIGITS, XRAM_DIGITS_PALETTE, MODE5_SIZE(8, 8), MODE5_1BPP},
    { 266, 10, XRAM_DIGITS+80, XRAM_DIGITS_PALETTE, MODE5_SIZE(8, 8), MODE5_1BPP},
    { 274, 10, XRAM_DIGITS, XRAM_DIGITS_PALETTE, MODE5_SIZE(8, 8), MODE5_1BPP},
    { 282, 10, XRAM_DIGITS, XRAM_DIGITS_PALETTE, MODE5_SIZE(8, 8), MODE5_1BPP}
};

uint8_t pad[4];

unsigned char dirs, jump, fire;

void reset_target();

void init_game() {
    game.space_key_down = 0;
    game.target_dx = 4;
    game.target_dy = -20;
    game.nb_shots = 20;
    game.hostage_step = 0;
    game.easter_state = 0;
    game.over = 0;
    game.nb_targets = 0;
    game.nb_targets_hit = 0;
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
    uint8_t tmp;

    if (game.nb_shots == 0 || game.nb_targets == 20) {
        game_over();
        return;
    }

    game.target_status = 0;
    game.nb_targets++;
    if (game.nb_targets == 10 || game.nb_targets == 20) {
        hud_sprite[TARGET_LAUNCHED_H].xram_sprite_ptr += 8;
        hud_sprite[TARGET_LAUNCHED_L].xram_sprite_ptr = XRAM_DIGITS;
    } else {
        hud_sprite[TARGET_LAUNCHED_L].xram_sprite_ptr += 8;
    }
    elts_sprite[TARGET_BOOM].x_pos_px = 0;
    elts_sprite[TARGET_BOOM].y_pos_px = 250;

    // Left side or right side?
    tmp = ria_attr_get(RIA_ATTR_LRAND);
    if ((tmp & 1) == 0) {
        elts_sprite[TARGET].x_pos_px = 0;
        game.target_dx = 4;
    } else {
        elts_sprite[TARGET].x_pos_px = 320;
        game.target_dx = -4;
    }

    // What height?
    tmp = ria_attr_get(RIA_ATTR_LRAND) & 0x7F;
    elts_sprite[TARGET].y_pos_px = 220 - tmp;

    // Angle
    tmp = ria_attr_get(RIA_ATTR_LRAND) & 0x1F;
    game.target_dy = -30 - tmp;
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

    if (game.target_dx == 4) {
        if (elts_sprite[TARGET].x_pos_px >= 316) {
            reset_target();
        }
    } else {
        if (elts_sprite[TARGET].x_pos_px == 0) {
            reset_target();
        }
    }
}

void shoot() {
    uint8_t tmp;

    if (game.nb_shots == 0) {
        return;
    }
    game.nb_shots--;
    if (game.nb_shots == 10) {
        game.easter_state = 0xFF;
    }
    hud_sprite[game.nb_shots].y_pos_px = 250;

    // if there is an easter egg
    if (game.easter_state != 0) {
        if (elts_sprite[CROSSHAIR].x_pos_px < elts_sprite[HOSTAGE].x_pos_px+10) {
            if (elts_sprite[CROSSHAIR].x_pos_px > elts_sprite[HOSTAGE].x_pos_px-24) {
                if (elts_sprite[CROSSHAIR].y_pos_px < elts_sprite[HOSTAGE].y_pos_px+6) {
                    if (elts_sprite[CROSSHAIR].y_pos_px > elts_sprite[HOSTAGE].y_pos_px-38) {
                        elts_sprite[HOSTAGE].xram_sprite_ptr = XRAM_HOSTAGE + 512;
                        elts_sprite[CROSSHAIR].y_pos_px = 250;
                        game_over();
                        return;
                    }
                }
            }
        }

        if (game.easter_state == 0xFF) {
            if (elts_sprite[CROSSHAIR].x_pos_px < elts_sprite[BAD_GUY].x_pos_px+10) {
                if (elts_sprite[CROSSHAIR].x_pos_px > elts_sprite[BAD_GUY].x_pos_px-24) {
                    if (elts_sprite[CROSSHAIR].y_pos_px < elts_sprite[BAD_GUY].y_pos_px+6) {
                        if (elts_sprite[CROSSHAIR].y_pos_px > elts_sprite[BAD_GUY].y_pos_px-38) {
                            elts_sprite[BAD_GUY].xram_sprite_ptr = XRAM_BAD_GUY + 1536;
                            game.easter_state = 40;
                            for (tmp=0; tmp<10; tmp++) {
                                hud_sprite[game.nb_shots].y_pos_px = 10;
                                game.nb_shots++;
                            }
                        }
                    }
                }
            }
        }
    }

    // Check if the target is hit
    if (elts_sprite[CROSSHAIR].x_pos_px > elts_sprite[TARGET].x_pos_px+10) return;
    if (elts_sprite[CROSSHAIR].x_pos_px < elts_sprite[TARGET].x_pos_px-24) return;
    if (elts_sprite[CROSSHAIR].y_pos_px > elts_sprite[TARGET].y_pos_px+6) return;
    if (elts_sprite[CROSSHAIR].y_pos_px < elts_sprite[TARGET].y_pos_px-22) return;

    // Hit the target
    game.nb_targets_hit++;
    if (game.nb_targets_hit == 10 || game.nb_targets_hit == 20) {
        hud_sprite[TARGET_HIT_H].xram_sprite_ptr += 8;
        hud_sprite[TARGET_HIT_L].xram_sprite_ptr = XRAM_DIGITS;
    } else {
        hud_sprite[TARGET_HIT_L].xram_sprite_ptr += 8;
    }

    game.target_status = 32;
    elts_sprite[TARGET_BOOM].x_pos_px = elts_sprite[TARGET].x_pos_px;
    elts_sprite[TARGET_BOOM].y_pos_px = elts_sprite[TARGET].y_pos_px;
    target_sprite.options = MODE5_1BPP;

    elts_sprite[TARGET].y_pos_px = 250;
//    xram0_write(XRAM_TARGET_SPRITE, &target_boom_sprite, sizeof(target_boom_sprite));
}

static keyboard_t k;

void handle_input() {
        xram0_read(&k, XRAM_KEYBOARD, sizeof(k));
        if (KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_LEFT)) {
            if (crosshair_sprite.x_pos_px) {
                    elts_sprite[CROSSHAIR].x_pos_px-=4;
                }
        } else if (KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_RIGHT)) {
            if (crosshair_sprite.x_pos_px < 288) {
                    elts_sprite[CROSSHAIR].x_pos_px+=4;
                }
        }

        if (KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_UP)) {
            if (crosshair_sprite.y_pos_px) {
                    elts_sprite[CROSSHAIR].y_pos_px-=4;
                }
        } else if (KEYBOARD_PRESSED(k.keys, HID_KEY_ARROW_DOWN)) {
            if (crosshair_sprite.y_pos_px < 224) {
                    elts_sprite[CROSSHAIR].y_pos_px+=4;
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

void init_npc() {

}

void game_over() {
    game.over = 1;
    hud_sprite[GAME_OVER1].y_pos_px = 88;
    hud_sprite[GAME_OVER2].y_pos_px = 88;
}

void move_npcs() {
    if (!game.easter_state) return;

    // If the hostage made it out, stop
    elts_sprite[HOSTAGE].x_pos_px--;
    if (elts_sprite[HOSTAGE].x_pos_px == 5) {
        elts_sprite[HOSTAGE].y_pos_px = 250;
        elts_sprite[BAD_GUY].y_pos_px = 250;
        game.easter_state = 0;
        return;
    }
    
    // Move hostage
    game.hostage_step++;
    if (game.hostage_step == 16) {
        elts_sprite[HOSTAGE].xram_sprite_ptr += 256;
    } else if (game.hostage_step == 32) {
        game.hostage_step = 0;
        elts_sprite[HOSTAGE].xram_sprite_ptr = XRAM_HOSTAGE;
    }

    // Move bad guy (if not shot)
    if (game.easter_state == 0xFF) {
        elts_sprite[BAD_GUY].x_pos_px--;
        game.bad_guy_step++;
        if ((game.bad_guy_step & 0x7) == 0) {
            elts_sprite[BAD_GUY].x_pos_px--;
        }
        if (game.bad_guy_step == 16 || game.bad_guy_step == 32) {
            elts_sprite[BAD_GUY].xram_sprite_ptr += 512;
        } else if (game.bad_guy_step == 48) {
            game.bad_guy_step = 0;
            elts_sprite[BAD_GUY].xram_sprite_ptr = XRAM_BAD_GUY;
        }
    } else {
        if (game.easter_state == 1) {
            elts_sprite[BAD_GUY].y_pos_px = 250;
        } else {
            game.easter_state--;
        }
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
    xreg_vga_mode5(MODE5_CUSTOM, XRAM_ELTS_SPRITE, 5, 1, 0, 0);
    xreg_vga_mode5(MODE5_CUSTOM, XRAM_HUD_SPRITE, 27, 2, 0, 0);
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

        if (!game.over) {
            move_target();
            handle_input();
            move_npcs();
        }

        // Update the sprites
        xram0_write(XRAM_ELTS_SPRITE, &elts_sprite, sizeof(elts_sprite));
        if (game.target_status) {
            game.target_status--;
            if (!game.target_status) {
                reset_target();
            }
        }

//        xram0_write(XRAM_CROSSHAIR_SPRITE, &crosshair_sprite, sizeof(crosshair_sprite));
        xram0_write(XRAM_HUD_SPRITE, &hud_sprite, sizeof(hud_sprite));
    };
}
