#ifndef SETTINGS_H
#define SETTINGS_H

#include "rotary_encoder.h"

#include <stdbool.h>

typedef struct {
  void (*redraw_fn)(bool /* hovered */, bool /* selected */);
  void (*changed_fn)(ReDirection /* direction the rotary encoder has been rotated towards */);
} SettingParams;

#define SETTINGS_COUNT 2

extern const SettingParams setting_params[SETTINGS_COUNT];

#endif