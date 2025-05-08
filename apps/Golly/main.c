#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "extapp_api.h"

#include "inc/peripherals.h"

#define SIZE 5

void extapp_main(void)
{
    waitForKeyReleased();
    clearscreen();

    Cell cells[SIZE][SIZE];
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            cells[i][j].nthGen = 0;
            cells[i][j].isAlive = false;
        }
    }

    waitForKeyPressed();
    waitForKeyReleased();

    return;
}
