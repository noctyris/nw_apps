#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "src/libspng/spng.h"
#include "extapp_api.h"

extern uint32_t _heap_size;
extern void *_heap_base;
extern void *_heap_ptr;

int get_current_image_size(const char* filename, struct spng_ihdr* ihdr) {
    size_t len;
    const char *file_content = extapp_fileRead(filename, &len, EXTAPP_FLASH_FILE_SYSTEM);
    if (!file_content) return -1;

    spng_ctx *ctx = spng_ctx_new(0);
    if (!ctx) return -1;

    spng_set_png_buffer(ctx, file_content, len);
    int result = spng_get_ihdr(ctx, ihdr);
    spng_ctx_free(ctx);

    return result;
}

void draw_screen(char* Filename, char* Dimension, int ImageNumber, int ImageTotal, int ToolbarPalette, int BackgroundPalette, int TextPalette) {
    extapp_pushRectUniform(0, 18, 320, 222, BackgroundPalette);
    extapp_pushRectUniform(0, 0, 320, 18, ToolbarPalette);
    if (ImageTotal > 1) {
        //Draw the arrows if there is more that one image.
        extapp_pushRectUniform(311, 114, 1, 13, ToolbarPalette);
        extapp_pushRectUniform(312, 115, 1, 11, ToolbarPalette);
        extapp_pushRectUniform(313, 116, 1, 9, ToolbarPalette);
        extapp_pushRectUniform(314, 117, 1, 7, ToolbarPalette);
        extapp_pushRectUniform(315, 118, 1, 5, ToolbarPalette);
        extapp_pushRectUniform(316, 119, 1, 3, ToolbarPalette);
        extapp_pushRectUniform(317, 120, 1, 1, ToolbarPalette);

        extapp_pushRectUniform(10, 114, 1, 13, ToolbarPalette);
        extapp_pushRectUniform(9, 115, 1, 11, ToolbarPalette);
        extapp_pushRectUniform(8, 116, 1, 9, ToolbarPalette);
        extapp_pushRectUniform(7, 117, 1, 7, ToolbarPalette);
        extapp_pushRectUniform(6, 118, 1, 5, ToolbarPalette);
        extapp_pushRectUniform(5, 119, 1, 3, ToolbarPalette);
        extapp_pushRectUniform(4, 120, 1, 1, ToolbarPalette);
    }
    //Draw the text on the screen.
    int FilenameLength = extapp_drawTextSmall(Filename, 0, 0, TextPalette, ToolbarPalette, true);
    extapp_drawTextSmall(Filename, (320/2) - (FilenameLength/2), 3, TextPalette, ToolbarPalette, false);
    extapp_drawTextSmall(Dimension, 7, 3, TextPalette, ToolbarPalette, false);
    char ImageNumberBuffer[16];
    sprintf(ImageNumberBuffer, "%d/%d", ImageNumber, ImageTotal);
    int ImageNumberLength = extapp_drawTextSmall(ImageNumberBuffer, 0, 0, TextPalette, ToolbarPalette, true);
    extapp_drawTextSmall(ImageNumberBuffer, 320 - 7 - ImageNumberLength, 3, TextPalette, ToolbarPalette, false);
}

// Function to check if an image is too large for the screen
int is_image_too_large(uint32_t width, uint32_t height) {
    return (width > 320 || height > 222);
}

// Function to check if zooming is possible
int can_zoom(uint32_t width, uint32_t height) {
    return !(width*2 > 320 || height*2 > 222);
}

// Function to decode
int decode_and_display_png(const char* filename, int ImageNum, int ImageTotal, int zoom, int ToolbarPalette, int BackgroundPalette, int TextPalette) {
    size_t len;
    const char *file_content = extapp_fileRead(filename, &len, EXTAPP_FLASH_FILE_SYSTEM);
    if (!file_content) {
        draw_screen(filename, " ", ImageNum + 1, ImageTotal, ToolbarPalette, BackgroundPalette, TextPalette);
        extapp_drawTextLarge("Failed to read file", 80, 111, TextPalette, BackgroundPalette, false);
        return 0;
    }

    spng_ctx *ctx = spng_ctx_new(0);
    if (!ctx) {
        draw_screen(filename, " ", ImageNum + 1, ImageTotal, ToolbarPalette, BackgroundPalette, TextPalette);
        extapp_drawTextLarge("Failed to create spng context", 80, 111, TextPalette, BackgroundPalette, false);
        return 0;
    }

    spng_set_png_buffer(ctx, file_content, len);

    struct spng_ihdr ihdr;
    if (spng_get_ihdr(ctx, &ihdr)) {
        spng_ctx_free(ctx);
        draw_screen(filename, " ", ImageNum + 1, ImageTotal, ToolbarPalette, BackgroundPalette, TextPalette);
        extapp_drawTextLarge("Failed to get image size", 80, 111, TextPalette, BackgroundPalette, false);
        return 0;
    }

    char DimensionBuffer[20];
    sprintf(DimensionBuffer, "%dx%d", ihdr.width, ihdr.height);
    draw_screen(filename, DimensionBuffer, ImageNum + 1, ImageTotal, ToolbarPalette, BackgroundPalette, TextPalette);

    // Check if the image is too large for the screen
    if (is_image_too_large(ihdr.width, ihdr.height)) {
        spng_ctx_free(ctx);
        draw_screen(filename, DimensionBuffer, ImageNum + 1, ImageTotal, ToolbarPalette, BackgroundPalette, TextPalette);
        extapp_drawTextLarge("Image is too large for screen", 55, 101, TextPalette, BackgroundPalette, false);
        char sizeWarning[40];
        sprintf(sizeWarning, "Max size: 320x222, Image: %dx%d", ihdr.width, ihdr.height);
        extapp_drawTextLarge(sizeWarning, 45, 121, TextPalette, BackgroundPalette, false);
        return 0;
    }

    // If zoom is requested but not possible, switch to normal mode
    if (zoom && !can_zoom(ihdr.width, ihdr.height)) {
        zoom = 0;
    }

    uint32_t display_width = zoom ? ihdr.width*2 : ihdr.width;
    uint32_t display_height = zoom ? ihdr.height*2 : ihdr.height;

    // Calculate offset for centering the image
    int OffsetX = (320 - display_width) / 2;
    int OffsetY = (220 - display_height) / 2 + 19;

    // Limit the size of our row buffer to conserve memory
    // Max 320 pixels wide, 4 rows at a time to save memory
    const int CHUNK_HEIGHT = 4;
    uint8_t *row_buffer = malloc(ihdr.width * CHUNK_HEIGHT * 3);

    if (!row_buffer) {
        spng_ctx_free(ctx);
        draw_screen(filename, DimensionBuffer, ImageNum + 1, ImageTotal, ToolbarPalette, BackgroundPalette, TextPalette);
        extapp_drawTextLarge("Failed to allocate memory", 80, 111, TextPalette, BackgroundPalette, false);
        return 0;
    }

    // Create a progressive decoder
    if (spng_decode_image(ctx, NULL, 0, SPNG_FMT_RGB8, SPNG_DECODE_PROGRESSIVE)) {
        free(row_buffer);
        spng_ctx_free(ctx);
        draw_screen(filename, DimensionBuffer, ImageNum + 1, ImageTotal, ToolbarPalette, BackgroundPalette, TextPalette);
        extapp_drawTextLarge("Failed to setup decoder", 80, 111, TextPalette, BackgroundPalette, false);
        return 0;
    }

    // Process the image in chunks
    for (uint32_t y = 0; y < ihdr.height; y += CHUNK_HEIGHT) {
        int current_chunk_height = (y + CHUNK_HEIGHT > ihdr.height) ? (ihdr.height - y) : CHUNK_HEIGHT;

        struct spng_row_info row_info;
        for (int i = 0; i < current_chunk_height; i++) {
            if (spng_get_row_info(ctx, &row_info)) {
                free(row_buffer);
                spng_ctx_free(ctx);
                return 0;
            }

            if (spng_decode_row(ctx, row_buffer + (i * ihdr.width * 3), ihdr.width * 3)) {
                free(row_buffer);
                spng_ctx_free(ctx);
                return 0;
            }
        }

        for (int i = 0; i < current_chunk_height; i++) {
            for (uint32_t x = 0; x < ihdr.width; x++) {
                uint8_t *pixel = row_buffer + (i * ihdr.width + x) * 3;
                // Convert the pixel to a rgb565 color
                uint16_t color = ((pixel[0] >> 3) << 11) | ((pixel[1] >> 2) << 5) | (pixel[2] >> 3);

                if (zoom) {
                    extapp_pushRectUniform(OffsetX + x*2, OffsetY + (y+i)*2, 2, 2, color);
                } else {
                    extapp_pushRectUniform(OffsetX + x, OffsetY + y+i, 1, 1, color);
                }
            }
        }
    }

    free(row_buffer);
    spng_ctx_free(ctx);
    return 1;
}

void extapp_main() {
    //Take the palette of the using the pull function.
    int ToolbarPalette;
    extapp_pullRect(0, 0, 1, 1, &ToolbarPalette);
    int BackgroundPalette;
    extapp_pullRect(0, 18, 1, 1, &BackgroundPalette);
    int TextPalette;
    extapp_pullRect(299, 7, 1, 1, &TextPalette);

    //Quit the app if the exam mode is enabled.
    if (extapp_inexammode()) {
        draw_screen("Exam mode is enabled", " ", 0, 0, ToolbarPalette, BackgroundPalette, TextPalette);
        extapp_drawTextLarge("Exam mode enabled", 80, 111, TextPalette, BackgroundPalette, false);
        extapp_drawTextLarge("App closing in 2 seconds.", 45, 131, TextPalette, BackgroundPalette, false);
        extapp_msleep(2000);
        return;
    }

    //list of the images
    const char* filenames[8];
    int NumberOfImage = extapp_fileListWithExtension(filenames, 8, ".png", EXTAPP_FLASH_FILE_SYSTEM);

    if (NumberOfImage == 0) {
        draw_screen("No images found", " ", 0, 0, ToolbarPalette, BackgroundPalette, TextPalette);
        extapp_drawTextLarge("No images found", 80, 111, TextPalette, BackgroundPalette, false);
        extapp_drawTextLarge("App closing in 2 seconds.", 45, 131, TextPalette, BackgroundPalette, false);
        extapp_msleep(2000);
        return;
    }

    int Loop = 1;
    int ImageNum = 0;
    int ImageTotal = NumberOfImage;
    int Drawn = 1;
    int Zoom = 0;

    while (Loop == 1) {
        if (extapp_isKeydown(5) || extapp_isKeydown(6) || extapp_isKeydown(7) || extapp_isKeydown(52)) {
            Loop = 0;
            break;
        }

        if (Drawn == 1) {
            draw_screen(filenames[ImageNum], "Loading...", ImageNum + 1, ImageTotal, ToolbarPalette, BackgroundPalette, TextPalette);
            Drawn = 0;
            decode_and_display_png(filenames[ImageNum], ImageNum, ImageTotal, Zoom, ToolbarPalette, BackgroundPalette, TextPalette);
        }

        if (extapp_isKeydown(3)) {
            if (ImageTotal > 1) {
                ImageNum++;
                if (ImageNum >= ImageTotal) {
                    ImageNum = 0;
                }
                Drawn = 1;
                extapp_msleep(200);
            }
        }
        if (extapp_isKeydown(0)) {
            if (ImageTotal > 1) {
                ImageNum--;
                if (ImageNum < 0) {
                    ImageNum = ImageTotal - 1;
                }
                Drawn = 1;
                extapp_msleep(200);
            }
        }
        if (extapp_isKeydown(46)) {
            if (Zoom == 1) {
                Zoom = 0;
                Drawn = 1;
                extapp_msleep(200);
            }
        }
        if (extapp_isKeydown(45) && Zoom == 0) {
            struct spng_ihdr ihdr;
            if (get_current_image_size(filenames[ImageNum], &ihdr) == 0) {
                if (can_zoom(ihdr.width, ihdr.height)) {
                    Zoom = 1;
                    Drawn = 1;
                }
            }
            extapp_msleep(200);
        }
    }
    return 0;
}