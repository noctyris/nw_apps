#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <complex.h>
#include <math.h>
#include "extapp_api.h"

#include "inc/peripherals.h"
#include "inc/menu.h"

void extapp_main(void)
{
    waitForKeyReleased();
    int quit = 0;

    int SIZE_X = 320, SIZE_Y = 240;
    int MAX_ITER;
    double complex c;

    menu(&MAX_ITER, &c, &quit);
    if (quit) return;
    clearscreen();

    for (int i = -(SIZE_X - SIZE_Y) / 2; i < SIZE_X - (SIZE_X - SIZE_Y) / 2; i++)
    {
        for (int j = 0; j < SIZE_Y; j++)
        {
            double x = -1.5 + (3.0 * i) / SIZE_Y;
            double y = -1.5 + (3.0 * j) / SIZE_Y;
            double complex z = x + y * I;
            int iter = 0;
            while (cabs(z) < 2 && iter < MAX_ITER)
            {
                z = z * z + c;
                iter++;
            }
            uint16_t color = makeColor(iter, MAX_ITER);
            extapp_pushRectUniform(i + (SIZE_X - SIZE_Y) / 2, j, 1, 1, color);
        }
    }

    waitForKeyReleased();
    waitForKeyPressed();
    waitForKeyReleased();

    return;
}
