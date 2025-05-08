#include "inc/menu.h"

#define DEFAULT rgb_16(166, 166, 166)
#define SELECTED 0xFFFF
#define NOPTIONS 2

void menu(int *MAX_ITER, double complex *c, int *quit)
{
    int lselect = 0;
    input_t inputs[NOPTIONS];
    inputs[0] = (input_t){.cursor = 0, .max = 30, .value = calloc(31, sizeof(char)), .x_pos = 65};
    inputs[1] = (input_t){.cursor = 0, .max = 3, .value = calloc(4, sizeof(char)), .x_pos = 135};
    uint64_t previous_scancode = 0;

    clearscreen();

    extapp_pushRectUniform(0, 0, 320, 18, rgb_16(50, 50, 50));
    extapp_drawTextSmall("FRACTAL", 140, 4, 0xFFFF, rgb_16(50, 50, 50), 0);

    extapp_drawTextLarge(lselect == 0 ? "> c =" : "  c =", 5, 40, lselect == 0 ? SELECTED : DEFAULT, BGCOL, 0);
    extapp_drawTextLarge(lselect == 1 ? "> Max. iter:" : "  Max. iter:", 5, 65, lselect == 1 ? SELECTED : DEFAULT, BGCOL, 0);
    extapp_drawTextSmall(inputs[0].value, inputs[0].x_pos, 44, DEFAULT, BGCOL, 0);
    extapp_drawTextSmall(inputs[1].value, inputs[1].x_pos, 69, SELECTED, BGCOL, 0);
    extapp_drawTextLarge("<Ok>", 270, 200, DEFAULT, BGCOL, 0);
    extapp_drawTextSmall("Toujours en noir et blanc", 5, 227, DEFAULT, BGCOL, 0);

    while (true)
    {
        for (int i = 0; i < NOPTIONS; i++)
        {
            inputs[i].value[inputs[i].cursor] = '\0';
        }

        extapp_drawTextLarge(lselect == 0 ? "> c =" : "  c =", 5, 40, lselect == 0 ? SELECTED : DEFAULT, BGCOL, 0);
        extapp_drawTextLarge(lselect == 1 ? "> Max. iter:" : "  Max. iter:", 5, 65, lselect == 1 ? SELECTED : DEFAULT, BGCOL, 0);
        extapp_drawTextSmall(inputs[0].value, inputs[0].x_pos, 44, lselect == 0 ? SELECTED : DEFAULT, BGCOL, 0);
        extapp_drawTextSmall(inputs[1].value, inputs[1].x_pos, 69, lselect == 1 ? SELECTED : DEFAULT, BGCOL, 0);

        uint64_t scancode = extapp_scanKeyboard();

        if (scancode != 0 && scancode != previous_scancode)
        {
            if (scancode & SCANCODE_Up) lselect = (lselect - 1 + NOPTIONS) % NOPTIONS;
            else if (scancode & SCANCODE_Down) lselect = (lselect + 1) % NOPTIONS;
            else if (scancode & SCANCODE_Back) {
                *quit = 1;
                break;
            }
            else if (scancode & SCANCODE_OK || scancode & SCANCODE_EXE) break;
            else if (scancode & SCANCODE_Backspace && inputs[lselect].cursor > 0)
            {
                inputs[lselect].cursor--;
                inputs[lselect].value[inputs[lselect].cursor] = '\0';
                extapp_pushRectUniform(inputs[lselect].x_pos + 7 * inputs[lselect].cursor, 44 + lselect * 25, 100, 18, BGCOL);
            }
            else if (inputs[lselect].cursor < inputs[lselect].max)
            {
                char _c = lselect == 1 ? scancodeToCharNbr(scancode) : scancodeToCharFull(scancode);
                if (_c != '\0')
                {
                    inputs[lselect].value[inputs[lselect].cursor] = _c;
                    inputs[lselect].cursor++;
                    extapp_pushRectUniform(inputs[lselect].x_pos + 7 * inputs[lselect].cursor, 44 + lselect * 25, 100, 18, BGCOL);
                    extapp_drawTextSmall(inputs[lselect].value, inputs[lselect].x_pos, 44 + lselect * 25, SELECTED, BGCOL, 0);
                }
            }
        }

        previous_scancode = scancode;

        extapp_msleep(10);
    }

    waitForKeyPressed();
    waitForKeyReleased();

    *MAX_ITER = atoi(inputs[1].value);
    if (*MAX_ITER < 1)
        *MAX_ITER = 100;
    *c = parse_complex(inputs[0].value);

    free(inputs[0].value);
    free(inputs[1].value);
}