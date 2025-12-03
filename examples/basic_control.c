// Demonstrates basic bulb operations using the cwiz library.

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

  printf("cwiz Example - Basic Bulb Control\n");
  printf("======================================\n\n");

  // Connect to bulb
  printf("Connecting to bulb at %s...\n", bulb_ip);
  wiz_bulb_t *bulb = wiz_bulb_create(bulb_ip);
  if (!bulb) {
    fprintf(stderr, "Error: Failed to create bulb connection\n");
    return 1;
  }

  // Turn on
  printf("\nTurning bulb ON...\n");
  int ret = wiz_bulb_turn_on(bulb);
  if (ret != WIZ_OK) {
    fprintf(stderr, "Error turning on bulb: %s\n", wiz_strerror(ret));
    wiz_bulb_destroy(bulb);
    return 1;
  }
  sleep(2);

  
  // Set color: Red
  printf("Setting color to RED...\n");
  ret = wiz_bulb_set_rgb(bulb, 255, 0, 0);
  if (ret != WIZ_OK) {
    fprintf(stderr, "Error setting RGB: %s\n", wiz_strerror(ret));
  }
  sleep(2);

  // Set color: Green
  printf("Setting color to GREEN...\n");
  ret = wiz_bulb_set_rgb(bulb, 0, 255, 0);
  if (ret != WIZ_OK) {
    fprintf(stderr, "Error setting RGB: %s\n", wiz_strerror(ret));
  }
  sleep(2);
  
  // Set color: Blue
  printf("Setting color to BLUE...\n");
  ret = wiz_bulb_set_rgb(bulb, 0, 0, 255);
  if (ret != WIZ_OK) {
    fprintf(stderr, "Error setting RGB: %s\n", wiz_strerror(ret));
  }
  sleep(2);
  
  // Set temperature: Warm White (2700K)
  printf("Setting warm white (2700K)...\n");
  ret = wiz_bulb_set_temperature(bulb, 2700);
  if (ret != WIZ_OK) {
    fprintf(stderr, "Error setting temperature: %s\n", wiz_strerror(ret));
  }
  sleep(2);
  
  // Set temperature: Cool White (6500K)
  printf("Setting cool white (6500K)...\n");
  ret = wiz_bulb_set_temperature(bulb, 6500);
  if (ret != WIZ_OK) {
    fprintf(stderr, "Error setting temperature: %s\n", wiz_strerror(ret));
  }
  sleep(2);

  // Set brightness to 50%
  printf("Setting brightness to 50%%...\n");
  ret = wiz_bulb_set_brightness(bulb, 50);
  if (ret != WIZ_OK) {
    fprintf(stderr, "Error setting brightness: %s\n", wiz_strerror(ret));
  }
  sleep(2);
    
  // Retrieve state
  printf("\nGetting bulb state...\n");
  ret = wiz_bulb_update_state(bulb);
  if (ret == WIZ_OK) {
    wiz_bulb_state_t state;
    wiz_bulb_get_state(bulb, &state);
    printf("  State: %s\n", state.state ? "ON" : "OFF");
    printf("  Brightness: %d\n", state.brightness);
    printf("  Temperature: %dK\n", state.temp);
    printf("  RGB: (%d, %d, %d)\n", state.rgb.r, state.rgb.g, state.rgb.b);
    printf("  RSSI: %d dBm\n", state.rssi);
  }

  // Turn off
  printf("\nTurning bulb OFF...\n");
  ret = wiz_bulb_turn_off(bulb);
  if (ret != WIZ_OK) {
    fprintf(stderr, "Error turning off bulb: %s\n", wiz_strerror(ret));
  }

  // Cleanup
  wiz_bulb_destroy(bulb);
  printf("\nExample complete!\n");

  return 0;
}
