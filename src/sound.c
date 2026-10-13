#include <rp6502.h>
#include <stddef.h>
#include <stdint.h>
#include "sound.h"
#include "xram.h"

void sound_init(void)
{
    xram0_set(XRAM_PSG, 0, sizeof(psg_t));
    xreg_ria_psg(XRAM_PSG);
}

void sound_start(uint16_t channel, const psg_channel_t *effect)
{
    xram0_poke8(channel + offsetof(psg_channel_t, pan_gate), 0);
    xram0_write(channel, effect, offsetof(psg_channel_t, pan_gate));
    xram0_poke8(channel + offsetof(psg_channel_t, pan_gate), PSG_GATE);
}

void sound_update()
{
    
}
