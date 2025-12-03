// Demonstrates setting various lighting scenes on a WiZ bulb.

#include "cwiz.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <bulb_ip_address> [scene_id]\n", argv[0]);
    printf("Example: %s 192.168.1.100 1\n\n", argv[0]);
    printf("If no scene_id provided, cycles through several scenes.\n");
    return 1;
  }

  const char *bulb_ip = argv[1];

  printf("cwiz Example - Scene Control\n");
  printf("=================================\n\n");

  // create bulb connection
  printf("Connecting to bulb at %s...\n", bulb_ip);
  wiz_bulb_t *bulb = wiz_bulb_create(bulb_ip);
  if (!bulb) {
    fprintf(stderr, "Error: Failed to create bulb connection\n");
    return 1;
  }

  // turn on bulb first
  printf("Turning bulb ON...\n");
  int ret = wiz_bulb_turn_on(bulb);
  if (ret != WIZ_OK) {
    fprintf(stderr, "Error turning on bulb: %s\n", wiz_strerror(ret));
    wiz_bulb_destroy(bulb);
    return 1;
  }
  sleep(1);

  // if specific scene requested, set it
  if (argc > 2) {
    uint16_t scene_id = atoi(argv[2]);
    const char *scene_name = wiz_get_scene_name(scene_id);

    if (scene_name) {
      printf("\nSetting scene: %s (ID: %d)\n", scene_name, scene_id);
      ret = wiz_bulb_set_scene(bulb, scene_id);
      if (ret != WIZ_OK) {
        fprintf(stderr, "Error setting scene: %s\n", wiz_strerror(ret));
      }
    } else {
      printf("Unknown scene ID: %d\n", scene_id);
    }
  } else {
    // cycle through several popular scenes
    uint16_t scenes[] = {1, 2, 3, 4, 6, 9, 10, 11, 12};
    int scene_count = sizeof(scenes) / sizeof(scenes[0]);

    printf("\nCycling through %d scenes...\n\n", scene_count);

    for (int i = 0; i < scene_count; i++) {
      uint16_t scene_id = scenes[i];
      const char *scene_name = wiz_get_scene_name(scene_id);

      printf("Setting scene: %s (ID: %d)\n", scene_name, scene_id);
      ret = wiz_bulb_set_scene(bulb, scene_id);
      if (ret != WIZ_OK) {
        fprintf(stderr, "Error setting scene: %s\n", wiz_strerror(ret));
      }

      sleep(3);
    }
  }

  // list all available scenes
  printf("\n\nAvailable Scenes:\n");
  printf("=================\n");

  int count;
  const wiz_scene_t *all_scenes = wiz_get_all_scenes(&count);

  for (int i = 0; i < count; i++) {
    printf("  %4d: %s\n", all_scenes[i].id, all_scenes[i].name);
  }

  // clean up
  wiz_bulb_destroy(bulb);
  printf("\nExample complete!\n");

  return 0;
}
