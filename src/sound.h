#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>
#include "xram.h"


#define PSG_WAVE_SINE 0x00
#define PSG_WAVE_SQUARE 0x10
#define PSG_WAVE_SAWTOOTH 0x20
#define PSG_WAVE_TRIANGLE 0x30
#define PSG_WAVE_NOISE 0x40

#define PSG_GATE 0x01
#define PSG_FREQ_HZ(hz) ((hz) * 3u)
#define PSG_PAN(pan) ((uint8_t)((pan) * 2))
#define HIT_CHANNEL (XRAM_PSG + offsetof(psg_t, channel[0]))

#define xreg_ria_psg(...) xreg(0, 1, 0, __VA_ARGS__)

static const psg_channel_t hit = {
    PSG_FREQ_HZ(800), 255, 0x00, 0xF6, 0x04 | PSG_WAVE_NOISE};

void sound_init(void);
void sound_start(uint16_t channel, const psg_channel_t *effect);
void sound_update(void);

#endif
