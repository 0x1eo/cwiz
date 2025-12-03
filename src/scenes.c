#include "../include/cwiz.h"
#include <stddef.h>
#include <string.h>

// scene database - ordered by ID
static const wiz_scene_t scenes[] = {
    {1, "Ocean"},        {2, "Romance"},       {3, "Sunset"},
    {4, "Party"},        {5, "Fireplace"},     {6, "Cozy"},
    {7, "Forest"},       {8, "Pastel colors"}, {9, "Wake-up"},
    {10, "Bedtime"},     {11, "Warm white"},   {12, "Daylight"},
    {13, "Cool white"},  {14, "Night light"},  {15, "Focus"},
    {16, "Relax"},       {17, "True colors"},  {18, "TV time"},
    {19, "Plantgrowth"}, {20, "Spring"},       {21, "Summer"},
    {22, "Fall"},        {23, "Deep dive"},    {24, "Jungle"},
    {25, "Mojito"},      {26, "Club"},         {27, "Christmas"},
    {28, "Halloween"},   {29, "Candlelight"},  {30, "Golden white"},
    {31, "Pulse"},       {32, "Steampunk"},    {33, "Diwali"},
    {34, "White"},       {35, "Alarm"},        {36, "Snowy sky"},
    {1000, "Rhythm"}};

static const int scene_count = sizeof(scenes) / sizeof(scenes[0]);

const char *wiz_get_scene_name(uint16_t scene_id) {
  for (int i = 0; i < scene_count; i++) {
    if (scenes[i].id == scene_id) {
      return scenes[i].name;
    }
  }
  return NULL;
}

uint16_t wiz_get_scene_id(const char *scene_name) {
  if (!scene_name)
    return 0;

  for (int i = 0; i < scene_count; i++) {
    if (strcmp(scenes[i].name, scene_name) == 0) {
      return scenes[i].id;
    }
  }
  return 0;
}

const wiz_scene_t *wiz_get_all_scenes(int *count) {
  if (count) {
    *count = scene_count;
  }
  return scenes;
}
