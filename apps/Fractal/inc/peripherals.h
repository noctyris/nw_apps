#ifndef CHIPEE_PERIPHERALS_H_
#define CHIPEE_PERIPHERALS_H_

#include <stdint.h>


extern long long unsigned int keymappings[16];
extern int keymappings_await[16];

void clearscreen();
void waitForKeyPressed();
void waitForKeyReleased();
void waitForKeyReleasedTimeout(int timeout);
uint16_t makeColor(int iter, int max);
uint16_t rgb_16(uint16_t r, uint16_t g, uint16_t b);

#endif
