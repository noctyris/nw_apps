#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "extapp_api.h"

#include "inc/peripherals.h"

void extapp_main(void)
{
    waitForKeyReleased();
    clearscreen();

    

    waitForKeyPressed();
    waitForKeyReleased();

    return;
}
