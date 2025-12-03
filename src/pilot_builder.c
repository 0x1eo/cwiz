#include "../include/cwiz.h"
#include <stdlib.h>
#include <string.h>

wiz_pilot_builder_t *wiz_pilot_builder_create(void) {
  wiz_pilot_builder_t *builder =
      (wiz_pilot_builder_t *)calloc(1, sizeof(wiz_pilot_builder_t));
  return builder;
}

void wiz_pilot_builder_destroy(wiz_pilot_builder_t *builder) {
  if (builder) {
    free(builder);
  }
}

void wiz_pilot_builder_set_state(wiz_pilot_builder_t *builder, bool state) {
  if (!builder)
    return;
  builder->has_state = true;
  builder->state = state;
}

void wiz_pilot_builder_set_brightness(wiz_pilot_builder_t *builder,
                                      uint8_t brightness) {
  if (!builder)
    return;

  // clamp brightness to valid range
  if (brightness < 10)
    brightness = 10;
  if (brightness > 100)
    brightness = 100;

  builder->has_brightness = true;
  builder->brightness = brightness;
}

void wiz_pilot_builder_set_rgb(wiz_pilot_builder_t *builder, uint8_t r,
                               uint8_t g, uint8_t b) {
  if (!builder)
    return;

  builder->has_rgb = true;
  builder->rgb.r = r;
  builder->rgb.g = g;
  builder->rgb.b = b;
}

void wiz_pilot_builder_set_temperature(wiz_pilot_builder_t *builder,
                                       uint16_t temp) {
  if (!builder)
    return;

  // clamp temperature to valid range
  if (temp < WIZ_TEMP_MIN)
    temp = WIZ_TEMP_MIN;
  if (temp > WIZ_TEMP_MAX)
    temp = WIZ_TEMP_MAX;

  builder->has_temp = true;
  builder->temp = temp;
}

void wiz_pilot_builder_set_scene(wiz_pilot_builder_t *builder,
                                 uint16_t scene_id) {
  if (!builder)
    return;

  builder->has_scene_id = true;
  builder->scene_id = scene_id;
}
