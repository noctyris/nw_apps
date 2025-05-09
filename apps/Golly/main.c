#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "extapp_api.h"

#include "inc/peripherals.h"

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

    drawCells(cells);
    drawCursor(cells, coord);

    while (running)
    {
        if (!paused)
        {
            uint64_t scancode = extapp_scanKeyboard();
            switch (scancode)
            {
            case SCANCODE_Home:
                running = false;
                break;
            case SCANCODE_Back:
                paused = true;
                drawCells(cells);
                drawCursor(cells, coord);
                waitForKeyReleased();
                break;
            case SCANCODE_Plus:
                speed = speed < 10 ? speed + 1 : speed;
                waitForKeyReleased();
                break;
            case SCANCODE_Minus:
                speed = speed > 1 ? speed - 1 : speed;
                waitForKeyReleased();
                break;
            case SCANCODE_Zero:
                initCells(cells);
                paused=true;
                drawCells(cells);
                drawCursor(cells, coord);
                break;
            }

            drawCells(cells);

            for (int i = 0; i < ROWS; i++)
            {
                for (int j = 0; j < COLUMNS; j++)
                {
                    int aliveNeighbors = countNeighbors(cells, i, j);
                    if (cells[(i+ROWS)%ROWS][(j+COLUMNS)%COLUMNS].isAlive)
                    {
                        newCells[(i+ROWS)%ROWS][(j+COLUMNS)%COLUMNS].isAlive = aliveNeighbors == 2 || aliveNeighbors == 3;
                        newCells[(i+ROWS)%ROWS][(j+COLUMNS)%COLUMNS].nthGen = cells[(i+ROWS)%ROWS][(j+COLUMNS)%COLUMNS].nthGen + 1;
                    }
                    else
                    {
                        newCells[(i+ROWS)%ROWS][(j+COLUMNS)%COLUMNS].isAlive = aliveNeighbors == 3;
                        newCells[(i+ROWS)%ROWS][(j+COLUMNS)%COLUMNS].nthGen = 1;
                    }
                }
            }

            for (int i = 0; i < ROWS; i++)
            {
                for (int j = 0; j < COLUMNS; j++)
                {
                    cells[i][j].isAlive = newCells[i][j].isAlive;
                    cells[i][j].nthGen = newCells[i][j].nthGen;
                }
            }

            extapp_msleep(-10*speed + 100);
            nGen++;
        }
        else
        {
            uint64_t scancode = extapp_scanKeyboard();
            switch (scancode)
            {
                case SCANCODE_Home:
                    running = false;
                    break;
                case SCANCODE_Back:
                    paused = false;
                    waitForKeyReleased();
                    break;
                case SCANCODE_Left:
                    coord.x = (coord.x - 1 + ROWS) % ROWS;
                    drawCells(cells);
                    drawCursor(cells, coord);
                    waitForKeyReleased();
                    break;
                case SCANCODE_Right:
                    coord.x = (coord.x + 1) % ROWS;
                    drawCells(cells);
                    drawCursor(cells, coord);
                    waitForKeyReleased();
                    break;
                case SCANCODE_Up:
                    coord.y = (coord.y - 1 + COLUMNS) % COLUMNS;
                    drawCells(cells);
                    drawCursor(cells, coord);
                    waitForKeyReleased();
                    break;
                case SCANCODE_Down:
                    coord.y = (coord.y + 1) % COLUMNS;
                    drawCells(cells);
                    drawCursor(cells, coord);
                    waitForKeyReleased();
                    break;
                case SCANCODE_OK:
                    cells[coord.x][coord.y].isAlive = !cells[coord.x][coord.y].isAlive;
                    cells[coord.x][coord.y].nthGen = cells[coord.x][coord.y].isAlive ? 1 : 0;
                    drawCells(cells);
                    drawCursor(cells, coord);
                    waitForKeyReleased();
                    break;
                case SCANCODE_Zero:
                    initCells(cells);
                    drawCells(cells);
                    drawCursor(cells, coord);
                    waitForKeyReleased();
                    break;
            }
            extapp_msleep(20);
        }
    }

    free(cells);
    free(newCells);

    return;
}
