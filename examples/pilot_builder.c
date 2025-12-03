// Demonstrates using the pilot builder to set multiple parameters at once.

#include "cwiz.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <bulb_ip_address>\n", argv[0]);
    printf("Example: %s 192.168.1.100\n", argv[0]);
    return 1;
  }

  const char *bulb_ip = argv[1];

  printf("cwiz Example - Pilot Builder\n");
  printf("==================================\n\n");

  // create bulb connection
  printf("Connecting to bulb at %s...\n", bulb_ip);
  wiz_bulb_t *bulb = wiz_bulb_create(bulb_ip);
  if (!bulb) {
    fprintf(stderr, "Error: Failed to create bulb connection\n");
    return 1;
  }

  // example 1: Turn on with brightness and color
  printf("\nExample 1: Turn on with purple color at 75%% brightness\n");
  wiz_pilot_builder_t *builder = wiz_pilot_builder_create();
  wiz_pilot_builder_set_state(builder, true);
  wiz_pilot_builder_set_brightness(builder, 75);
  wiz_pilot_builder_set_rgb(builder, 128, 0, 255); // purple

  int ret = wiz_bulb_apply_pilot(bulb, builder);
  if (ret != WIZ_OK) {
    fprintf(stderr, "Error applying pilot: %s\n", wiz_strerror(ret));
  } else {
    printf("Successfully applied settings\n");
  }
  wiz_pilot_builder_destroy(builder);
  sleep(3);

  // example 2: Set warm white with brightness
  printf("\nExample 2: Set warm white (2700K) at full brightness\n");
  builder = wiz_pilot_builder_create();
  wiz_pilot_builder_set_brightness(builder, 100);
  wiz_pilot_builder_set_temperature(builder, 2700);

  ret = wiz_bulb_apply_pilot(bulb, builder);
  if (ret != WIZ_OK) {
    fprintf(stderr, "Error applying pilot: %s\n", wiz_strerror(ret));
  } else {
    printf("Successfully applied settings\n");
  }
  wiz_pilot_builder_destroy(builder);
  sleep(3);

  // example 3: Set orange color with medium brightness
  printf("\nExample 3: Set orange color at medium brightness\n");
  builder = wiz_pilot_builder_create();
  wiz_pilot_builder_set_brightness(builder, 50);
  wiz_pilot_builder_set_rgb(builder, 255, 165, 0); // orange

  ret = wiz_bulb_apply_pilot(bulb, builder);
  if (ret != WIZ_OK) {
    fprintf(stderr, "Error applying pilot: %s\n", wiz_strerror(ret));
  } else {
    printf("Successfully applied settings\n");
  }
  wiz_pilot_builder_destroy(builder);
  sleep(3);

  // example 4: Set scene with brightness
  printf("\nExample 4: Set 'Ocean' scene (ID: 1) at 50%% brightness\n");
  builder = wiz_pilot_builder_create();
  wiz_pilot_builder_set_brightness(builder, 50);
  wiz_pilot_builder_set_scene(builder, 1);

  ret = wiz_bulb_apply_pilot(bulb, builder);
  if (ret != WIZ_OK) {
    fprintf(stderr, "Error applying pilot: %s\n", wiz_strerror(ret));
  } else {
    printf("Successfully applied settings\n");
  }
  wiz_pilot_builder_destroy(builder);
  sleep(3);

  // turn off
  printf("\nTurning bulb off...\n");
  builder = wiz_pilot_builder_create();
  wiz_pilot_builder_set_state(builder, false);
  wiz_bulb_apply_pilot(bulb, builder);
  wiz_pilot_builder_destroy(builder);

  // clean up
  wiz_bulb_destroy(bulb);
  printf("\nExample complete!\n");

  return 0;
}
