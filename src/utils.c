#include "../include/cwiz.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

const char *wiz_strerror(int error) {
  switch (error) {
  case WIZ_OK:
    return "Success";
  case WIZ_ERR_SOCKET:
    return "Socket error";
  case WIZ_ERR_TIMEOUT:
    return "Timeout";
  case WIZ_ERR_INVALID_PARAM:
    return "Invalid parameter";
  case WIZ_ERR_JSON_PARSE:
    return "JSON parse error";
  case WIZ_ERR_NO_RESPONSE:
    return "No response from bulb";
  case WIZ_ERR_MALLOC:
    return "Memory allocation failed";
  case WIZ_ERR_CONNECTION:
    return "Connection error";
  default:
    return "Unknown error";
  }
}

void wiz_rgb_to_rgbcw(wiz_rgb_t rgb, uint16_t temp, wiz_rgbcw_t *rgbcw) {
  if (!rgbcw)
    return;

  // simple conversion - cool white and warm white based on temperature
  rgbcw->r = rgb.r;
  rgbcw->g = rgb.g;
  rgbcw->b = rgb.b;

  if (temp <= WIZ_TEMP_MIN) {
    rgbcw->w = 255;
    rgbcw->c = 0;
  } else if (temp >= WIZ_TEMP_MAX) {
    rgbcw->w = 0;
    rgbcw->c = 255;
  } else {
    // integer math interpolation
    uint32_t range = WIZ_TEMP_MAX - WIZ_TEMP_MIN;
    uint32_t val = temp - WIZ_TEMP_MIN;
    
    rgbcw->c = (uint8_t)((val * 255) / range);
    rgbcw->w = 255 - rgbcw->c;
  }
}

int wiz_hex_to_rgb(const char *hex_color, wiz_rgb_t *rgb) {
  if (!hex_color || !rgb) {
    return WIZ_ERR_INVALID_PARAM;
  }

  if (hex_color[0] == '#') {
    hex_color++;
  }

  if (strlen(hex_color) != 6) {
    return WIZ_ERR_INVALID_PARAM;
  }

  for (int i = 0; i < 6; i++) {
    if (!isxdigit(hex_color[i])) {
      return WIZ_ERR_INVALID_PARAM;
    }
  }

  unsigned int r, g, b;
  if (sscanf(hex_color, "%02x%02x%02x", &r, &g, &b) != 3) {
    return WIZ_ERR_INVALID_PARAM;
  }

  rgb->r = (uint8_t)r;
  rgb->g = (uint8_t)g;
  rgb->b = (uint8_t)b;

  return WIZ_OK;
}
