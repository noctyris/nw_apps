#ifndef MENU_H
#define MENU_H

#include "extapp_api.h"
#include "peripherals.h"

typedef struct {
    char *value;
    int cursor;
    int max;
    int x_pos;
} input_t;

void menu(int *MAX_ITER, double complex *c, int *quit);

#endif