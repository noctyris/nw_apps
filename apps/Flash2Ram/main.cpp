#include <cstdint>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <extapp_api.h>

#include "inc/peripherals.h"
#include "inc/selector.h"

extern "C" void extapp_main();

void extapp_main(void) {

    // TODO: Get top bar color

    // Wait for the key to be released before starting the application
    Peripherals::waitForKeyReleased();
    // Draw a white background
    Peripherals::init_display();


    // Select a file to copy
    const char * filename = Selector::select_file("", 100);
    // Wait for the key to be released
    Peripherals::waitForKeyReleased();

    // Clear the screen
    Peripherals::init_display();

    // If no file was selected, exit the application
    if (filename == NULL) {
        Peripherals::init_display();
        extapp_drawTextLarge("No file selected !", 0, 20 * 1, 0x0000, 0xFFFF, false);
        extapp_drawTextLarge("Press any key to exit", 0, 20 * 2, 0x0000, 0xFFFF, false);
        Peripherals::waitForKeyPressed();
        Peripherals::waitForKeyReleased();
        return;
    }

    // Show the selected file name
    extapp_drawTextLarge("Copying :", 0, 20 * 1, 0x0000, 0xFFFF, false);
    // If file is a Python file, we use a "hack" to add the autoimportation bit
    // to Python files

    extapp_drawTextLarge(filename, 0, 20 * 2, 0x0000, 0xFFFF, false);


    // Initialize the file len variable that will be written by the file read function
    size_t file_len = 0;
    // Read the file into filecontent variable
    const char * filecontent = extapp_fileRead(filename, &file_len, EXTAPP_FLASH_FILE_SYSTEM);

    const char * extension = strrchr(filename, '.');
    bool success = false;
    if (strcmp(extension, "py") != 0) {
        success = extapp_fileWrite(filename, filecontent - 1, file_len + 1, EXTAPP_RAM_FILE_SYSTEM);
        if (success) {
            // Here, we edit the first byte to \0
            char * ramfilecontent = (char *)extapp_fileRead(filename, &file_len, EXTAPP_FLASH_FILE_SYSTEM);
            ramfilecontent[0] = '\0';
        }
    } else {
        success = extapp_fileWrite(filename, filecontent, file_len, EXTAPP_RAM_FILE_SYSTEM);
    }

    // Write the file
    if (!success) {
        Peripherals::init_display();
        extapp_drawTextLarge("Error when copying file !", 0, 20 * 1, 0x0000, 0xFFFF, false);
        extapp_drawTextLarge("Press any key to exit", 0, 20 * 2, 0x0000, 0xFFFF, false);
        Peripherals::waitForKeyPressed();
        Peripherals::waitForKeyReleased();
        return;
    }
    extapp_drawTextLarge("File copied successfully !", 0, 20 * 5, 0x0000, 0xFFFF, false);

    extapp_drawTextLarge("Press any key to exit", 0, 20 * 6, 0x0000, 0xFFFF, false);
    // Wait for a key to be pressed before exiting the application
    Peripherals::waitForKeyPressed();
    Peripherals::waitForKeyReleased();
}
