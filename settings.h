#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>

typedef struct {
  void (*redraw_fn)(bool /* selected */);
} SettingParams;

#define SETTINGS_COUNT 2

extern const SettingParams setting_params[SETTINGS_COUNT];

#endif
