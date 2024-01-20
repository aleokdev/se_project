#include "settings.h"
#include "ssd1306.h"

void redraw_volume_setting(bool selected);
void redraw_tone_setting(bool selected);

const SettingParams setting_params[SETTINGS_COUNT] = {
                                                      { .redraw_fn = redraw_volume_setting },
                                                      { .redraw_fn = redraw_tone_setting }
};

void redraw_volume_setting(bool selected) {
    ssd1306_printText(3*6, 1, "Volume", selected);
    ssd1306_printText(6 * 10, 1, "------*--", false);
}

void redraw_tone_setting(bool selected) {
    ssd1306_printText(6*5, 2, "Tone", selected);
    ssd1306_printText(6 * 10, 2, "460 Hz", false);
}
