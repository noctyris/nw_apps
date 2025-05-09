#include "inc/pausedMode.h"
#include "extapp_api.h"

// Gestion du mode édition (paused)
void handlePausedMode(Cell(*cells)[COLUMNS], coord_t *coord, bool *running, bool *paused, int *speed)
{
    uint64_t scancode = extapp_scanKeyboard();
    switch (scancode)
    {
    case SCANCODE_Home:
        *running = false; // Quitter le programme
        break;
    case SCANCODE_Back:
        *paused = false; // Démarrer la simulation
        waitForKeyReleased();
        break;
    case SCANCODE_Left:
        coord->x = (coord->x - 1 + ROWS) % ROWS;
        drawCellsMono(cells);
        drawCursor(cells, *coord);
        waitForKeyReleased();
        break;
    case SCANCODE_Right:
        coord->x = (coord->x + 1) % ROWS;
        drawCellsMono(cells);
        drawCursor(cells, *coord);
        waitForKeyReleased();
        break;
    case SCANCODE_Up:
        coord->y = (coord->y - 1 + COLUMNS) % COLUMNS;
        drawCellsMono(cells);
        drawCursor(cells, *coord);
        waitForKeyReleased();
        break;
    case SCANCODE_Down:
        coord->y = (coord->y + 1) % COLUMNS;
        drawCellsMono(cells);
        drawCursor(cells, *coord);
        waitForKeyReleased();
        break;
    case SCANCODE_OK:
        cells[coord->x][coord->y].isAlive = !cells[coord->x][coord->y].isAlive;
        cells[coord->x][coord->y].nthGen = cells[coord->x][coord->y].isAlive ? 1 : 0;
        drawCellsMono(cells);
        drawCursor(cells, *coord);
        waitForKeyReleased();
        break;
    case SCANCODE_Zero:
        initCells(cells);
        drawCellsMono(cells);
        drawCursor(cells, *coord);
        *speed = 1;
        waitForKeyReleased();
        break;
    }

    extapp_msleep(20);
}