#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <complex.h>
#include "extapp_api.h"
#include "peripherals.h"

typedef struct {
    char *value;
    int cursor;
    int max;
    int x_pos;
} input_t;

void menu(int *MAX_ITER, double complex *c);

#endif