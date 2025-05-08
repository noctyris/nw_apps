#ifndef CHIPEE_PERIPHERALS_H_
#define CHIPEE_PERIPHERALS_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <complex.h>
#include <ctype.h>

#define BGCOL 0x0000

extern long long unsigned int keymappings[16];
extern int keymappings_await[16];

void clearscreen();
void waitForKeyPressed();
void waitForKeyReleased();
void waitForKeyReleasedTimeout(int timeout);
uint16_t makeColor(int iter, int max);
uint16_t rgb_16(uint16_t r, uint16_t g, uint16_t b);
char scancodeToCharFull(uint64_t scancode);
char scancodeToCharNbr(uint64_t scancode);
double complex parse_complex(const char *str);

#endif
