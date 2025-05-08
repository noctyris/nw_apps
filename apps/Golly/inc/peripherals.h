#ifndef CHIPEE_PERIPHERALS_H_
#define CHIPEE_PERIPHERALS_H_

#include <stdint.h>


extern long long unsigned int keymappings[16];
extern int keymappings_await[16];

void clearscreen();
void waitForKeyPressed();
void waitForKeyReleased();
void waitForKeyReleasedTimeout(int timeout);

#endif
