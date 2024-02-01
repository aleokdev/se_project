#pragma once

#include "menus.h"

#include <stdbool.h>
#include <stdint.h>

#define MESSAGE_BUFFER_MAX_LENGTH 10

typedef struct {
  Menu menu_open;
  uint8_t setting_hovered;
  bool setting_is_selected;
} State;
