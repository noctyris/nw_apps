#include "inc/peripherals.h"
#include "inc/menu.h"

void menu(int *MAX_ITER, double complex *c) {
    char max_tmp[5], c_tmp[30];
    int max_cursor = 0, c_cursor = 0;

    clearscreen();

    // Draw the menu
    extapp_pushRectUniform(0, 0, 320, 18, rgb_16(50, 50, 50));
    extapp_drawTextSmall("FRACTAL", 140, 4, 0xFFFF, rgb_16(50, 50, 50), 0);

    waitForKeyPressed();
    waitForKeyReleased();
    *MAX_ITER = 100;
    *c = -0.5251993 - 0.5251993 * I;
}