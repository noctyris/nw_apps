#include <extapp_api.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "selector.h"
#include "peanut_gb.h"
#include "lz4.h"

#define MAX_SCRIPTSTORE_SIZE 8192

#define SAVE_COOLDOWN 120

#define NW_LCD_WIDTH 320
#define NW_LCD_HEIGHT 240

#define DUMMY_ROM 0
#define DUMMY_ROM_NAME Tetris

#define ENABLE_FRAME_LIMITER 1
#define TARGET_FRAME_DURATION 16
#define AUTOMATIC_FRAME_SKIPPING 1
// Useful when AUTOMATIC_FRAME_SKIPPING is disabled
#define FRAME_SKIPPING_DEFAULT_STATE false


#ifndef DUMMY_ROM
#define DUMMY_ROM 0
#endif

#if DUMMY_ROM
#define _DUMMY_ROM_VAR(NAME)   NAME ## _GB
#define DUMMY_ROM_VAR(NAME)    _DUMMY_ROM_VAR(NAME)
#define _DUMMY_ROM_FILE(NAME)   #NAME ".gb"
#define DUMMY_ROM_FILE(NAME)    _DUMMY_ROM_FILE(NAME)

#include "out.h"
#endif

static bool running = false;

struct priv_t {
  // Pointer to allocated memory holding GB file.
  const uint8_t *rom;
  // Pointer to allocated memory holding save file.
  uint8_t *cart_ram;
  // Line buffer
  uint16_t line_buffer[LCD_WIDTH];
};

// Returns a byte from the ROM file at the given address.
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr) {
  const struct priv_t * const p = gb->direct.priv;
  return p->rom[addr];
}

// Returns a byte from the cartridge RAM at the given address.
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr) {
  const struct priv_t * const p = gb->direct.priv;
  return p->cart_ram[addr];
}

// Writes a given byte to the cartridge RAM at the given address.
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr, const uint8_t val) {
  const struct priv_t * const p = gb->direct.priv;
  p->cart_ram[addr] = val;
}

// Ignore all errors.
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t val) {

  const char* gb_err_str[4] = {
    "UNKNOWN",
    "INVALID OPCODE",
    "INVALID READ",
    "INVALID WRITE"
  };

  switch (gb_err) {
    case GB_INVALID_WRITE:
    case GB_INVALID_READ:
      return;
    default:
      running = false;
  }

  // TODO: Handle errors.
}
const uint16_t palette_peanut_GB[4] = {0x9DE1, 0x8D61, 0x3306, 0x09C1};
const uint16_t palette_original[4] = {0x8F80, 0x24CC, 0x4402, 0x0A40};
const uint16_t palette_gray[4] = {0xFFFF, 0xAD55, 0x52AA, 0x0000};
const uint16_t palette_gray_negative[4] = {0x0000, 0x52AA, 0xAD55, 0xFFFF};
const uint16_t * palette = palette_peanut_GB;

inline uint16_t color_from_gb_pixel(uint8_t gb_pixel) {
    uint8_t gb_color = gb_pixel & 0x3;
    return palette[gb_color];
}

void lcd_draw_line_centered(struct gb_s *gb, const uint8_t pixels[LCD_WIDTH], const uint_fast8_t line) {
  struct priv_t *priv = gb->direct.priv;

  #pragma unroll 40
  for(unsigned int x = 0; x < LCD_WIDTH; x++) {
    priv->line_buffer[x] = color_from_gb_pixel(pixels[x]);
  }

  extapp_pushRect((NW_LCD_WIDTH - LCD_WIDTH) / 2, (NW_LCD_HEIGHT - LCD_HEIGHT) / 2 + line, LCD_WIDTH, 1, priv->line_buffer);
}

void lcd_draw_line_dummy(struct gb_s *gb, const uint8_t pixels[LCD_WIDTH], const uint_fast8_t line) {}

static void lcd_draw_line_maximized_ratio(struct gb_s * gb, const uint8_t * input_pixels, const uint_fast8_t line) {
  // Nearest neighbor scaling of a 160x144 texture to a 266x240 resolution (to keep the ratio)
  // Horizontally, we multiply by 1.66 (160*1.66 = 266)
  uint16_t output_pixels[266];

  #pragma unroll 40
  for (int i=0; i<LCD_WIDTH; i++) {
    uint16_t color = color_from_gb_pixel(input_pixels[i]);
    // We can't use floats for performance reason, so we use a fixed point
    // representation
    output_pixels[166*i/100] = color;
    // This line is useless 1/3 times, but using an if is slower
    output_pixels[166*i/100+1] = color;
  }

  // Vertically, we want to scale by a 5/3 ratio. So we need to make 5 lines out of three:  we double two lines out of three.
  uint16_t y = (5*line)/3;
  extapp_pushRect((NW_LCD_WIDTH - 265) / 2, y, 265, 1, output_pixels);
  if (line%3 != 0) {
    extapp_pushRect((NW_LCD_WIDTH - 265) / 2, y+1, 265, 1, output_pixels);
  }
}

enum save_status_e {
  SAVE_READ_OK,
  SAVE_WRITE_OK,
  SAVE_READ_ERR,
  SAVE_WRITE_ERR,
  SAVE_COMPRESS_ERR,
  SAVE_NODISP
};
static enum save_status_e saveMessage = SAVE_NODISP;

char* read_save_file(const char* name, size_t size) {
  char* save_name = malloc(strlen(name) + 3);
  strcpy(save_name, name);
  osd_newextension(save_name, ".gbs");

  char* output = malloc(size);

  if (extapp_fileExists(save_name, EXTAPP_RAM_FILE_SYSTEM)) {
    size_t file_len = 0;
    const char* save_content = extapp_fileRead(save_name, &file_len, EXTAPP_RAM_FILE_SYSTEM);
    int error = LZ4_decompress_safe(save_content, output, file_len, size);

    // Handling corrupted save.
    if (error <= 0) {
      memset(output, 0xFF, size);
      extapp_fileErase(save_name, EXTAPP_RAM_FILE_SYSTEM);
      saveMessage = SAVE_READ_ERR;
    } else {
      saveMessage = SAVE_READ_OK;
    }
  } else {
    memset(output, 0xFF, size);
  }

  free(save_name);

  return output;
}

void write_save_file(const char* name, char* data, size_t size) {
  char* save_name = malloc(strlen(name) + 3);
  strcpy(save_name, name);
  osd_newextension(save_name, ".gbs");

  char* output = malloc((size_t) MAX_SCRIPTSTORE_SIZE);

  int compressed_size = LZ4_compress_default(data, output, size, MAX_SCRIPTSTORE_SIZE);

  if (compressed_size > 0) {
    if (extapp_fileWrite(save_name, output, compressed_size, EXTAPP_RAM_FILE_SYSTEM)) {
      saveMessage = SAVE_WRITE_OK;
    } else {
      saveMessage = SAVE_WRITE_ERR;
    }
  } else {
    saveMessage = SAVE_COMPRESS_ERR;
  }

  free(save_name);
  free(output);
}

static bool wasSavePressed = false;
static bool wasMSpFPressed = false;
static uint8_t saveCooldown = 0;
static bool MSpFfCounter = false;

void extapp_main() {
  struct gb_s gb;
  enum gb_init_error_e gb_ret;
  struct priv_t priv = {
    .rom = NULL,
    .cart_ram = NULL
  };
  enum gb_init_error_e ret;

  #if DUMMY_ROM
  priv.rom = DUMMY_ROM_VAR(DUMMY_ROM_NAME);
  const char * file_name = DUMMY_ROM_FILE(DUMMY_ROM_NAME);
  #else
  const char * file_name = select_rom();
  if (!file_name)
    return;

  size_t file_len = 0;
  priv.rom = (const uint8_t*) extapp_fileRead(file_name, &file_len, EXTAPP_FLASH_FILE_SYSTEM);
  #endif

  // Alloc internal RAM.
  gb.wram = malloc(WRAM_SIZE);
  gb.vram = malloc(VRAM_SIZE);
  gb.hram = malloc(HRAM_SIZE);
  gb.oam = malloc(OAM_SIZE);

  gb_ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write, &gb_error, &priv);

  // TODO: Handle init errors.
  switch(gb_ret) {
    case GB_INIT_NO_ERROR:
      break;
    default:
      return;
  }

  // Alloc and init save RAM.
  size_t save_size = gb_get_save_size(&gb);
  priv.cart_ram = read_save_file(file_name, save_size);
  saveCooldown = SAVE_COOLDOWN;

  // Init LCD
  gb_init_lcd(&gb, &lcd_draw_line_centered);

  extapp_pushRectUniform(0, 0, NW_LCD_WIDTH, NW_LCD_HEIGHT, 0);

  uint32_t lastMSpF = 0;

  #if ENABLE_FRAME_LIMITER
  // We use a "smart" frame limiter: for each frame, we add
  // `frame duration - target frame time` to our budget. If the frame was faster
  // than target, we sleep for (simplified version without taking the case where
  // time budget > target frame time - last frame duration):
  // target frame time - last frame duration - time budget
  // This way, we will keep an average frame duration consistant.
  uint32_t timeBudget = 0;
  #endif

  // Skip 1/2 frame, spare 3 ms/f on my N0110
  bool frameSkipping = FRAME_SKIPPING_DEFAULT_STATE;
  void * drawLineMode = lcd_draw_line_centered;

  running = true;
  while(running) {
    uint64_t start = extapp_millis();
    uint64_t kb = extapp_scanKeyboard();

    gb.direct.joypad_bits.a = (kb & SCANCODE_Back) ? 0 : 1;
    gb.direct.joypad_bits.b = (kb & SCANCODE_OK) ? 0 : 1;
    gb.direct.joypad_bits.select = (kb & ((uint64_t)1 << 8)) ? 0 : 1;
    gb.direct.joypad_bits.start = (kb & SCANCODE_Home) ? 0 : 1;
    gb.direct.joypad_bits.up = (kb & SCANCODE_Up) ? 0 : 1;
    gb.direct.joypad_bits.right = (kb & SCANCODE_Right) ? 0 : 1;
    gb.direct.joypad_bits.left = (kb & SCANCODE_Left) ? 0 : 1;
    gb.direct.joypad_bits.down = (kb & SCANCODE_Down) ? 0 : 1;

    if (kb & SCANCODE_Backspace)
      gb_reset(&gb);
    if (kb & SCANCODE_Toolbox) {
      if (!wasSavePressed && saveCooldown == 0) {
        write_save_file(file_name, priv.cart_ram, save_size);
        saveCooldown = SAVE_COOLDOWN;
        wasSavePressed = true;
      }
    } else if (wasSavePressed) {
      wasSavePressed = false;
    }

    if (kb & SCANCODE_Alpha) {
      if (!wasMSpFPressed) {
        MSpFfCounter = !MSpFfCounter;
        wasMSpFPressed = true;
        extapp_pushRectUniform(0, NW_LCD_HEIGHT / 2 + LCD_HEIGHT / 2, NW_LCD_WIDTH, NW_LCD_HEIGHT - (NW_LCD_HEIGHT / 2 + LCD_HEIGHT / 2), 0);
      }
    } else if (wasMSpFPressed) {
      wasMSpFPressed = false;
    }

    if (kb & SCANCODE_Zero) {
      running = false;
      break;
    }

    if (kb & SCANCODE_Plus) {
      gb.display.lcd_draw_line = lcd_draw_line_maximized_ratio;
      drawLineMode = lcd_draw_line_maximized_ratio;
    }
    if (kb & SCANCODE_Minus) {
      gb.display.lcd_draw_line = lcd_draw_line_centered;
      drawLineMode = lcd_draw_line_centered;
      extapp_pushRectUniform(0, 0, NW_LCD_WIDTH, NW_LCD_HEIGHT, 0);
    }
    // if (kb & SCANCODE_Division) {
    //   gb.display.lcd_draw_line = lcd_draw_line_dummy;
    //   drawLineMode = lcd_draw_line_dummy;
    //   extapp_pushRectUniform(0, 0, NW_LCD_WIDTH, NW_LCD_HEIGHT, 0);
    // }

    if (kb & SCANCODE_One) {
      palette = palette_peanut_GB;
    }
    if (kb & SCANCODE_Two) {
      palette = palette_original;
    }
    if (kb & SCANCODE_Three) {
      palette = palette_gray;
    }
    if (kb & SCANCODE_Four) {
      palette = palette_gray_negative;
    }

    gb.gb_frame = 0;
    int i = 0;
    for(i = 0; !gb.gb_frame && i < 32000; i++)
      __gb_step_cpu(&gb);

    if (saveCooldown > 1) {
      saveCooldown--;
      switch(saveMessage) {
        case SAVE_READ_OK:
          extapp_drawTextSmall("Loaded save!", 10, NW_LCD_HEIGHT - 30, 65535, 0, false);
          break;
        case SAVE_READ_ERR:
          extapp_drawTextSmall("Error while loading save!", 10, NW_LCD_HEIGHT - 30, 65535, 0, false);
          break;
        case SAVE_WRITE_OK:
          extapp_drawTextSmall("Saved!", 10, NW_LCD_HEIGHT - 30, 65535, 0, false);
          break;
        case SAVE_WRITE_ERR:
          extapp_drawTextSmall("Error while writing save!", 10, NW_LCD_HEIGHT - 30, 65535, 0, false);
          break;
        case SAVE_COMPRESS_ERR:
          extapp_drawTextSmall("Error while compressing save!", 10, NW_LCD_HEIGHT - 30, 65535, 0, false);
          break;
        default:
          break;
      }
    } else if (saveCooldown == 1) {
      saveCooldown--;
      extapp_pushRectUniform(0, NW_LCD_HEIGHT / 2 + LCD_HEIGHT / 2, NW_LCD_WIDTH, NW_LCD_HEIGHT - (NW_LCD_HEIGHT / 2 + LCD_HEIGHT / 2), 0);
    }
    uint64_t end = extapp_millis();

    if (gb.gb_frame) {
      uint16_t MSpF = (uint16_t)(end - start);

      if (MSpFfCounter) {
        // We need to average the MSpF as skipped frames are faster
        uint16_t MSpFAverage = (MSpF + lastMSpF) / 2;
        char buffer[100];
        sprintf(buffer, "%d ms/f", MSpFAverage);
        // sprintf(buffer, "%d ms/f, %d ", MSpFAverage, timeBudget);
        extapp_drawTextSmall(buffer, 2, NW_LCD_HEIGHT - 10, 65535, 0, false);
      }

      if (frameSkipping) {
        if (gb.display.lcd_draw_line != lcd_draw_line_dummy) {
          drawLineMode = gb.display.lcd_draw_line;
          gb.display.lcd_draw_line = lcd_draw_line_dummy;
        } else {
          gb.display.lcd_draw_line = drawLineMode;
        }
      }

      #if ENABLE_FRAME_LIMITER
      uint32_t differenceToTarget = abs(TARGET_FRAME_DURATION - MSpF);

      if (TARGET_FRAME_DURATION - MSpF > 0) {
        // Frame was faster than target, so let's slow down if we have time to
        // catch up

        // If on previous frames we were
        if (timeBudget >= differenceToTarget) {
          // We were too slow at previous frames so we have to catch up
          timeBudget -= differenceToTarget;
        } else if (timeBudget > 0) {
          // We can catch up everything on one frame, so let's sleep a bit less
          // than what we would have done if we weren't late
          uint32_t time_to_sleep = differenceToTarget - timeBudget;
          extapp_msleep(time_to_sleep);
          timeBudget = 0;
        } else {
          // We don't have time to catch up, so we just sleep until we get to 16ms/f
          extapp_msleep(differenceToTarget);

          #if AUTOMATIC_FRAME_SKIPPING
          // Disable frame skipping as we are running faster than required
          frameSkipping = false;
          gb.display.lcd_draw_line = drawLineMode;
          #endif
        }
      } else {
        // Comparaison is technically not required, but we do this avoid the
        // performance cost of duplicate assignation when we are at the maximum
        // time budget, which is often the case when lagging
        if (timeBudget < TARGET_FRAME_DURATION) {
          // Frame was slower than target, so we need to catch up.
          timeBudget += differenceToTarget;

          if (timeBudget >= TARGET_FRAME_DURATION) {
            timeBudget = TARGET_FRAME_DURATION;

            #if AUTOMATIC_FRAME_SKIPPING
            // Enable frame skipping in an attempt to speed up emulation
            frameSkipping = true;
            #endif
          }
        }
      }
      #endif
      lastMSpF = MSpF;
    }
  }

  free(gb.wram);
  free(gb.vram);
  free(gb.hram);
  free(gb.oam);

  write_save_file(file_name, priv.cart_ram, save_size);
  free(priv.cart_ram);
}
