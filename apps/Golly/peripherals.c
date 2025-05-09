#include "inc/peripherals.h"
#include "extapp_api.h"
#include <string.h>

/**
 * init_display: clear the screen
 */
void clearscreen()
{
  // Draw a rectangle to fill all the screen
  extapp_pushRectUniform(0, 0, 320, 240, 0);
}

/**
 * waitForKeyPressed: wait for a key to be pressed
 */
void waitForKeyPressed()
{
  // Scan the keyboard until we get a pressed key
  while (!extapp_scanKeyboard())
  {
    // Sleep 10 milliseconds
    extapp_msleep(10);
  }
}

/**
 * waitForKeyReleased: wait for no keys pressed
 */
void waitForKeyReleased()
{
  // Scan the keyboard until we get no keys pressed
  while (extapp_scanKeyboard())
  {
    extapp_msleep(10);
  }
}

/**
 * waitForKeyReleasedTimeout: wait for no keys pressed, but with a timeout
 * @param timeout int
 */
void waitForKeyReleasedTimeout(int timeout)
{
  // Scan the keyboard until we get no keys pressed, but exit if the timeout reached
  while (extapp_scanKeyboard() && timeout > 0)
  {
    // Sleep 10 milliseconds
    extapp_msleep(10);
    // Decrease the timeout of 10 milliseconds
    timeout -= 10;
  }
}

uint16_t rgb_16(uint8_t r, uint8_t g, uint8_t b)
{
  // Convert RGB to 16-bit color
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/**
 * getColor: get the color of a cell
 * @param cell Cell
 */
uint16_t getColor(Cell cell)
{
  return !cell.isAlive ? 0x0000 : (cell.nthGen>5 ? 0xF000 : rgb_16(41*cell.nthGen + 50, 0, -41*cell.nthGen + 255));
}

/**
 * initCells: init all cells to starting point
 * @param cells Cell*
*/
void initCells(Cell (*cells)[COLUMNS])
{
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLUMNS; j++)
    {
      cells[i][j].nthGen = 0;
      cells[i][j].isAlive = false;
    }
  }
}

/**
 * drawCellsColor: draw the cells on the screen
 * @param cells Cell
 */
void drawCellsColor(Cell cells[ROWS][COLUMNS])
{
  // Draw the cells on the screen
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLUMNS; j++)
    {
      extapp_pushRectUniform(i * SIZE, j * SIZE, SIZE, SIZE, getColor(cells[i][j]));
    }
  }
}

/**
 * drawCellsMono: draw the cells on the screen
 * @param cells Cell
 */
void drawCellsMono(Cell cells[ROWS][COLUMNS])
{
  // Draw the cells on the screen
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLUMNS; j++)
    {
      extapp_pushRectUniform(i * SIZE, j * SIZE, SIZE, SIZE, cells[i][j].isAlive ? 0xFFFF : 0x0000);
    }
  }
}

int countNeighbors(Cell cells[ROWS][COLUMNS], int x, int y)
{
  // Count the number of alive neighbors
  int count = 0;
  for (int i = -1; i <= 1; i++)
  {
    for (int j = -1; j <= 1; j++)
    {
      if (i == 0 && j == 0)
        continue;
      int nx = (x + i + ROWS) % ROWS;
      int ny = (y + j + COLUMNS) % COLUMNS;
      if (nx < 0 || nx >= ROWS || ny < 0 || ny >= COLUMNS)
        continue;
      if (cells[nx][ny].isAlive)
        count++;
    }
  }
  return count;
}

void drawCursor(Cell cells[ROWS][COLUMNS], coord_t coord)
{
  // Draw the cursor on the screen
  extapp_pushRectUniform(coord.x * SIZE, coord.y * SIZE, SIZE, SIZE, 0xFF00);
  extapp_pushRectUniform(coord.x * SIZE + 1, coord.y * SIZE + 1, SIZE - 2, SIZE - 2, cells[coord.x][coord.y].isAlive ? 0xFFFF : 0x0000);
}