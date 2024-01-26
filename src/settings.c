#include "settings.h"

#include "msp430.h"

#define DEFAULT_SETTINGS (Settings) {                 \
    /* Default tone frequency: 12000 / 20 = 600 Hz */ \
    .tone_value = 19,                                 \
    /* Default tone volume: About half-way */         \
    .tone_volume = 3,                                 \
    /* Default dah time: 200 / 1500 * 1000 = 130ms */ \
    .dah_time = 199                                   \
}

Settings settings = DEFAULT_SETTINGS;

void reset_settings(void) {
    settings = DEFAULT_SETTINGS;
}

#define SETTINGS_PASSWORD 87
#define PASSWORD_ADDR (void*)(0x1080)
#define SETTINGS_ADDR (void*)(0x1080+2)
_Static_assert((uint16_t)SETTINGS_ADDR+sizeof(settings) < 0x10BF, "Settings struct does not fit in Flash Segment B");

void load_settings(void) {
    if(*(uint8_t*)PASSWORD_ADDR != SETTINGS_PASSWORD) {
        // Password bits set on segment B of the Flash Information Memory.
        // If these aren't equal to SETTINGS_PASSWORD, it means that setting values haven't
        // been initialized, so we shouldn't read them.
        reset_settings();
    } else {
        // Read settings
        memcpy(&settings, SETTINGS_ADDR, sizeof(settings));
    }
}

void save_settings(void) {
    // First erase the data currently present in segment B
    FCTL1 = FWKEY | ERASE;       // Erase individual segment only
    FCTL2 = FWKEY | FSSEL_1 | 35;// MCLK (16 MHz), divide by 35: ~457kHz
    FCTL3 = FWKEY | LOCKA;       // Disable flash-wide write lock, enable segment A lock for safety purposes
    *(uint8_t*)PASSWORD_ADDR = 0;// Clear segment B
    // Now save the current settings
    FCTL1 = FWKEY | WRT;         // Do not erase, bit/byte/word write mode
    *(uint8_t*)PASSWORD_ADDR = SETTINGS_PASSWORD; // Write settings password
    memcpy(SETTINGS_ADDR, &settings, sizeof(settings)); // Write settings
    FCTL1 = FWKEY;               // Do not erase, write disabled
    FCTL3 = FWKEY | LOCK;        // Enable flash-wide write lock
}
