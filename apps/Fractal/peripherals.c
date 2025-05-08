#include "inc/peripherals.h"
#include "extapp_api.h"
#include <string.h>

/**
 * clearscreen: clear the screen
 */
void clearscreen()
{
  // Draw a rectangle to fill all the screen
  extapp_pushRectUniform(0, 0, 320, 240, BGCOL);
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

/**
 * makeColor: convert RGB color to 16B color
 * @param iter nth iteration
 * @param max maximum iteration for the fractal
 */
uint16_t makeColor(int iter, int max)
{
  int intensity = (iter * 255) / max;

  uint16_t color = (intensity >> 3) << 11 | (intensity >> 2) << 5 | intensity >> 3;

  return color;
}

/**
 * rgb_16: convert RGB color to 16B color
 * @param r red value
 * @param g green value
 * @param b blue value
 */
uint16_t rgb_16(uint16_t r, uint16_t g, uint16_t b)
{
  uint16_t color = (r >> 3) << 11 | (g >> 2) << 5 | b >> 3;
  return color;
}

/**
 * scancodeToChar: convert a scancode to a char (all numeric keyboard)
 * @param scancode scancode obtained from extapp_scanKeyboard()
 */
char scancodeToCharFull(uint64_t scancode)
{
  switch (scancode)
  {
  case SCANCODE_Zero:
    return '0';
  case SCANCODE_One:
    return '1';
  case SCANCODE_Two:
    return '2';
  case SCANCODE_Three:
    return '3';
  case SCANCODE_Four:
    return '4';
  case SCANCODE_Five:
    return '5';
  case SCANCODE_Six:
    return '6';
  case SCANCODE_Seven:
    return '7';
  case SCANCODE_Eight:
    return '8';
  case SCANCODE_Nine:
    return '9';
  case SCANCODE_Minus:
    return '-';
  case SCANCODE_Plus:
    return '+';
  case SCANCODE_Dot:
    return '.';
  case SCANCODE_Imaginary:
    return 'i';
  default:
    return '\0'; // Return null character for unrecognized scancode
  }
}

/**
 * scancodeToChar: convert a scancode to a char (numbers only)
 * @param scancode scancode obtained from extapp_scanKeyboard()
 */
char scancodeToCharNbr(uint64_t scancode)
{
  switch (scancode)
  {
  case SCANCODE_Zero:
    return '0';
  case SCANCODE_One:
    return '1';
  case SCANCODE_Two:
    return '2';
  case SCANCODE_Three:
    return '3';
  case SCANCODE_Four:
    return '4';
  case SCANCODE_Five:
    return '5';
  case SCANCODE_Six:
    return '6';
  case SCANCODE_Seven:
    return '7';
  case SCANCODE_Eight:
    return '8';
  case SCANCODE_Nine:
    return '9';
  default:
    return '\0'; // Return null character for unrecognized scancode
  }
}

double complex parse_complex(const char *str)
{
  double real = 0.0, imag = 0.0;
  char *endptr;

  // Étape 1 : Extraire la partie réelle
  real = strtod(str, &endptr);

  // Étape 2 : Vérifier si une partie imaginaire existe
  if (*endptr == '+' || *endptr == '-')
  {
    // Avancer au-delà du signe
    const char *imag_start = endptr;

    // Extraire la partie imaginaire
    imag = strtod(imag_start, &endptr);

    // Vérifier si le caractère suivant est 'i' ou 'j'
    if (*endptr == 'i' || *endptr == 'j')
    {
      // Tout est valide, continuer
    }
    else
    {
      fprintf(stderr, "Erreur : Format de nombre complexe invalide.\n");
      return 0.0 + 0.0 * I; // Retourner un nombre complexe nul en cas d'erreur
    }
  }
  else if (*endptr == 'i' || *endptr == 'j')
  {
    // Cas spécial : Pas de partie réelle, seulement une partie imaginaire
    imag = real; // La valeur extraite est en fait la partie imaginaire
    real = 0.0;
  }

  // Retourner le nombre complexe
  return real + imag * I;
}