#ifndef XRAM_H
#define XRAM_H

#include <rp6502.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define xreg_vga_canvas(...) xreg(1, 0, 0, __VA_ARGS__)

#define CANVAS_CONSOLE 0
#define CANVAS_320X240 1
#define CANVAS_320X180 2
#define CANVAS_640X480 3
#define CANVAS_640X360 4

#define COLOR_FROM_RGB8(r, g, b) \
    ((((unsigned)(b) >> 3) << 11) | (((unsigned)(g) >> 3) << 6) | ((unsigned)(r) >> 3))
#define COLOR_FROM_RGB5(r, g, b) \
    (((unsigned)(b) << 11) | ((unsigned)(g) << 6) | (unsigned)(r))
#define COLOR_ALPHA_MASK (1u << 5)

#define xreg_vga_mode3(...) xreg(1, 0, 1, 3, __VA_ARGS__)
#define xreg_vga_mode5(...) xreg(1, 0, 1, 5, __VA_ARGS__)

#define MODE3_1BPP 0x00
#define MODE3_2BPP 0x01
#define MODE3_4BPP 0x02
#define MODE3_8BPP 0x03
#define MODE3_16BPP 0x04
#define MODE3_REVERSE_BITS 0x08

#define MODE5_1BPP 0x00
#define MODE5_2BPP 0x01
#define MODE5_4BPP 0x02
#define MODE5_8BPP 0x03
#define MODE5_8X8 0x00
#define MODE5_16X16 0x08
#define MODE5_32X32 0x10
#define MODE5_64X64 0x18
#define MODE5_128X128 0x20
#define MODE5_256X256 0x28
#define MODE5_512X512 0x30
#define MODE5_CUSTOM 0x38

#define GAMEPAD_PLAYERS 4

#define GAMEPAD_DPAD_UP 0x01
#define GAMEPAD_DPAD_DOWN 0x02
#define GAMEPAD_DPAD_LEFT 0x04
#define GAMEPAD_DPAD_RIGHT 0x08

#define GAMEPAD_FEAT_TYPE_MASK 0x30
#define GAMEPAD_TYPE_UNKNOWN 0x00
#define GAMEPAD_TYPE_WESTERN 0x10
#define GAMEPAD_TYPE_EASTERN 0x20
#define GAMEPAD_TYPE_PLAYSTATION 0x30
#define GAMEPAD_FEAT_STICKS 0x40
#define GAMEPAD_FEAT_CONNECTED 0x80

#define GAMEPAD_LSTICK_UP 0x01
#define GAMEPAD_LSTICK_DOWN 0x02
#define GAMEPAD_LSTICK_LEFT 0x04
#define GAMEPAD_LSTICK_RIGHT 0x08
#define GAMEPAD_RSTICK_UP 0x10
#define GAMEPAD_RSTICK_DOWN 0x20
#define GAMEPAD_RSTICK_LEFT 0x40
#define GAMEPAD_RSTICK_RIGHT 0x80

#define GAMEPAD_BTN0_A 0x01
#define GAMEPAD_BTN0_B 0x02
#define GAMEPAD_BTN0_C 0x04
#define GAMEPAD_BTN0_X 0x08
#define GAMEPAD_BTN0_Y 0x10
#define GAMEPAD_BTN0_Z 0x20
#define GAMEPAD_BTN0_L1 0x40
#define GAMEPAD_BTN0_R1 0x80

#define GAMEPAD_BTN1_L2 0x01
#define GAMEPAD_BTN1_R2 0x02
#define GAMEPAD_BTN1_SELECT 0x04
#define GAMEPAD_BTN1_START 0x08
#define GAMEPAD_BTN1_HOME 0x10
#define GAMEPAD_BTN1_L3 0x20
#define GAMEPAD_BTN1_R3 0x40

typedef struct
{
    bool x_wrap;
    bool y_wrap;
    int16_t x_pos_px;
    int16_t y_pos_px;
    int16_t width_tiles;
    int16_t height_tiles;
    uint16_t xram_data_ptr;
    uint16_t xram_palette_ptr;
    uint16_t xram_tile_ptr;
} mode2_config_t; /* layout */

typedef struct
{
    bool x_wrap;
    bool y_wrap;
    int16_t x_pos_px;
    int16_t y_pos_px;
    int16_t width_px;
    int16_t height_px;
    uint16_t xram_data_ptr;
    uint16_t xram_palette_ptr;
} mode3_config_t; /* layout */

typedef struct
{
    int16_t x_pos_px;
    int16_t y_pos_px;
    uint16_t xram_sprite_ptr;
    uint16_t palette_ptr;
    uint8_t width_height;
    uint8_t options;
} mode5_csprite_t; /* layout */

#define KEYBOARD_NO_KEY 0
#define KEYBOARD_NUM_LOCK 1
#define KEYBOARD_CAPS_LOCK 2
#define KEYBOARD_SCROLL_LOCK 3

#define KEYBOARD_PRESSED(keys, code) ((keys)[(code) >> 3] & (1 << ((code) & 7)))

#define HID_KEY_ARROW_RIGHT 0x4F
#define HID_KEY_ARROW_LEFT 0x50
#define HID_KEY_ARROW_DOWN 0x51
#define HID_KEY_ARROW_UP 0x52
#define HID_KEY_SPACE 0x2C

#define xreg_ria_keyboard(...) xreg(0, 0, 0, __VA_ARGS__)
#define xreg_ria_gamepad(...) xreg(0, 0, 2, __VA_ARGS__)

#define MODE5_SIZE(width, height) \
    ((((height) / 4 - 1) << 4) | ((width) / 4 - 1))

typedef struct
{
    uint8_t keys[32];
} keyboard_t; /* layout */

typedef struct
{
    uint16_t background[16];
    uint16_t splash[16];
    uint16_t game_over[16];
    uint16_t target[16];
    uint16_t target_boom[2];
    uint16_t crosshair[2];
    uint16_t shell[16];
    uint16_t hostage[16];
    uint16_t bad_guy[16];
    uint16_t digits[2];
    uint16_t difficulty[4];
} palettes_t;


_Static_assert(sizeof(palettes_t) / sizeof(uint16_t) <= 512,
               "palettes_t holds more than 512 colors.");

typedef struct
{
    uint8_t dpad;
    uint8_t sticks;
    uint8_t btn0;
    uint8_t btn1;
    int8_t lx;
    int8_t ly;
    int8_t rx;
    int8_t ry;
    uint8_t l2;
    uint8_t r2;
} gamepad_player_t;

typedef struct
{
    gamepad_player_t player[4];
} gamepad_t; /* layout */

typedef struct
{
    uint16_t freq;
    uint8_t duty;
    uint8_t attack;
    uint8_t decay;
    uint8_t release_wave;
    uint8_t pan_gate;
    uint8_t reserved;
} psg_channel_t;

typedef struct
{
    psg_channel_t channel;
} psg_t; /* layout */

typedef struct
{
    psg_t psg;
    palettes_t palettes;
    uint8_t background[320UL * 240 / 2];
    uint8_t target[16*16/2];
    uint8_t target_boom[16*16/8];
    uint8_t crosshair[32*32/8];
    uint8_t game_over1[56*64/2];
    uint8_t game_over2[56*64/2];
    uint8_t shell[8*16/2];
    uint8_t hostage[48*32/2];
    uint8_t bad_guy[128*32/2];
    uint8_t digits[88*8/8];
    uint8_t diff_easy[48*12/4];
    uint8_t diff_medium[48*12/4];
    uint8_t diff_hard[48*12/4];
    mode3_config_t background_config;
    mode5_csprite_t hud_sprite[27];
    mode5_csprite_t elts_sprite[6];
    keyboard_t keyboard;
    gamepad_t gamepad;
} xram_layout_t;

#define XRAM_PSG offsetof(xram_layout_t, psg)

#define XRAM_BACKGROUND offsetof(xram_layout_t, background)
#define XRAM_BACKGROUND_CONFIG offsetof(xram_layout_t, background_config)
#define XRAM_BACKGROUND_PALETTE offsetof(xram_layout_t, palettes.background)
#define XRAM_SPLASH_PALETTE offsetof(xram_layout_t, palettes.splash)
#define XRAM_DIFF_PALETTE offsetof(xram_layout_t, palettes.difficulty)
#define XRAM_DIFF_EASY offsetof(xram_layout_t, diff_easy)
#define XRAM_DIFF_MEDIUM offsetof(xram_layout_t, diff_medium)
#define XRAM_DIFF_HARD offsetof(xram_layout_t, diff_hard)

#define XRAM_CROSSHAIR offsetof(xram_layout_t, crosshair)
#define XRAM_CROSSHAIR_PALETTE offsetof(xram_layout_t, palettes.crosshair)

#define XRAM_TARGET offsetof(xram_layout_t, target)
#define XRAM_TARGET_PALETTE offsetof(xram_layout_t, palettes.target)

#define XRAM_TARGET_BOOM offsetof(xram_layout_t, target_boom)
#define XRAM_TARGET_BOOM_PALETTE offsetof(xram_layout_t, palettes.target_boom)

#define XRAM_SHELL offsetof(xram_layout_t, shell)
#define XRAM_SHELL_PALETTE offsetof(xram_layout_t, palettes.shell)

#define XRAM_HOSTAGE offsetof(xram_layout_t, hostage)
#define XRAM_HOSTAGE_PALETTE offsetof(xram_layout_t, palettes.hostage)

#define XRAM_BAD_GUY offsetof(xram_layout_t, bad_guy)
#define XRAM_BAD_GUY_PALETTE offsetof(xram_layout_t, palettes.bad_guy)

#define XRAM_GAME_OVER1 offsetof(xram_layout_t, game_over1)
#define XRAM_GAME_OVER2 offsetof(xram_layout_t, game_over2)
#define XRAM_GAME_OVER1_PALETTE offsetof(xram_layout_t, palettes.game_over)

#define XRAM_DIGITS offsetof(xram_layout_t, digits)
#define XRAM_DIGITS_PALETTE offsetof(xram_layout_t, palettes.digits)

#define XRAM_HUD_SPRITE offsetof(xram_layout_t, hud_sprite)
#define XRAM_ELTS_SPRITE offsetof(xram_layout_t, elts_sprite)

#define XRAM_GAMEPAD offsetof(xram_layout_t, gamepad)
#define XRAM_KEYBOARD offsetof(xram_layout_t, keyboard)

#endif
