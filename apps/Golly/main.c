#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "extapp_api.h"

#include "inc/peripherals.h"
#include "inc/pausedMode.h"
#include "inc/runningMode.h"

void extapp_main(void)
{
    waitForKeyReleased();
    clearscreen();

    bool running = true;
    bool paused = true;
    int nGen = 0;
    int speed = 1;
    coord_t coord = {ROWS/2, COLUMNS/2};

    Cell(*cells)[COLUMNS] = malloc(ROWS * sizeof(*cells));
    Cell(*newCells)[COLUMNS] = malloc(ROWS * sizeof(*newCells));

    if (!cells || !newCells) return;

    initCells(cells);

    // for (int i = 0; i < 500; i++)
    // {
    //     int x = rand() % ROWS;
    //     int y = rand() % COLUMNS;
    //     cells[x][y].isAlive = true;
    // }

    drawCellsColor(cells);
    drawCursor(cells, coord);

    while (running)
    {
        if (!paused) handleRunningMode(cells, newCells, &paused, &speed, &nGen);
        else handlePausedMode(cells, &coord, &running, &paused, &speed);
    }

    free(cells);
    free(newCells);

    return;
}
