#ifndef __SILVOS_AC97_H
#define __SILVOS_AC97_H

#include <stdint.h>

void ac97_device_register(uint8_t bus, uint8_t device, uint8_t function);

int ac97_enqueue_audio_out(const void* samples);

#endif
