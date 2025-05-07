#include "inc/peripherals.h"
#include "extapp_api.h"
#include <string.h>


/**
 * clearscreen: clear the screen
 */
void clearscreen() {
  // Draw a rectangle to fill all the screen
  extapp_pushRectUniform(0, 0, 320, 240, 0x0000);
}

/**
 * waitForKeyPressed: wait for a key to be pressed
 */
void waitForKeyPressed() {
  // Scan the keyboard until we get a pressed key
  while (!extapp_scanKeyboard()) {
    // Sleep 10 milliseconds
    extapp_msleep(10);
  }
}

/**
 * waitForKeyReleased: wait for no keys pressed
 */
void waitForKeyReleased() {
  // Scan the keyboard until we get no keys pressed
  while (extapp_scanKeyboard()) {
    extapp_msleep(10);
  }
}

/**
 * waitForKeyReleasedTimeout: wait for no keys pressed, but with a timeout
 * @param timeout int
 */
void waitForKeyReleasedTimeout(int timeout) {
  // Scan the keyboard until we get no keys pressed, but exit if the timeout reached
  while (extapp_scanKeyboard() && timeout > 0) {
    // Sleep 10 milliseconds
    extapp_msleep(10);
    // Decrease the timeout of 10 milliseconds
    timeout -= 10;
  }
}

/**
 * makeColor: convert RGB color to 16B color
 * @param iter nth iteration
 * @param max maximum iteration for the fractal
*/
uint16_t makeColor(int iter, int max) {
    int intensity = (iter * 255) / max;
    
    uint16_t color = (intensity>>3)<<11 | (intensity>>2)<<5 | intensity>>3;

    return color;
}

uint16_t rgb_16(uint16_t r, uint16_t g, uint16_t b) {
    uint16_t color = (r>>3)<<11 | (g>>2)<<5 | b>>3;
    return color;
}