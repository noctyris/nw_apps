#ifndef CHIPEE_PERIPHERALS_H_
#define CHIPEE_PERIPHERALS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 5
#define ROWS (320 / SIZE)
#define COLUMNS (240 / SIZE)

typedef struct {
    int nthGen;
    bool isAlive;
} Cell;

typedef struct {
    int x;
    int y;
} coord_t;

extern long long unsigned int keymappings[16];
extern int keymappings_await[16];

void clearscreen();
void waitForKeyPressed();
void waitForKeyReleased();
void waitForKeyReleasedTimeout(int timeout);
void drawCellsColor(Cell cells[ROWS][COLUMNS]);
void drawCellsMono(Cell cells[ROWS][COLUMNS]);
void initCells(Cell (*cells)[COLUMNS]);
int countNeighbors(Cell cells[ROWS][COLUMNS], int x, int y);
void drawCursor(Cell cells[ROWS][COLUMNS], coord_t coord);
uint16_t getColor(Cell cell);

#endif
