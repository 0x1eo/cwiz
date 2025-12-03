#include "../include/cwiz.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


extern int wiz_create_socket(void);
extern int wiz_send_receive(int sock, struct sockaddr_in *addr,
                            const char *message, char *response,
                            size_t response_size);
extern int wiz_build_json_message(char *buffer, size_t size, const char *method,
                                  const char *params);
extern int wiz_parse_get_pilot_response(const char *json,
                                        wiz_bulb_state_t *state);
extern int wiz_parse_system_config(const char *json, wiz_bulb_info_t *info);

// internal helper to send parameter update
static int _wiz_send_param(wiz_bulb_t *bulb, const char *method, const char *params) {
  char message[256];
  char response[1024];

  int ret = wiz_build_json_message(message, sizeof(message), method, params);
  if (ret != WIZ_OK) return ret;

  return wiz_send_receive(bulb->socket_fd, &bulb->addr, message, response, sizeof(response));
}

wiz_bulb_t *wiz_bulb_create(const char *ip_address) {
  if (!ip_address) {
    return NULL;
  }

  wiz_bulb_t *bulb = (wiz_bulb_t *)calloc(1, sizeof(wiz_bulb_t));
  if (!bulb) {
    return NULL;
  }

  strncpy(bulb->ip_address, ip_address, sizeof(bulb->ip_address) - 1);
  bulb->port = WIZ_PORT;

  bulb->socket_fd = wiz_create_socket();
  if (bulb->socket_fd < 0) {
    free(bulb);
    return NULL;
  }

  memset(&bulb->addr, 0, sizeof(bulb->addr));
  bulb->addr.sin_family = AF_INET;
  bulb->addr.sin_port = htons(bulb->port);

  if (inet_pton(AF_INET, ip_address, &bulb->addr.sin_addr) <= 0) {
    close(bulb->socket_fd);
    free(bulb);
    return NULL;
  }

  return bulb;
}

void wiz_bulb_destroy(wiz_bulb_t *bulb) {
  if (!bulb)
    return;

  if (bulb->socket_fd >= 0) {
    close(bulb->socket_fd);
  }

  free(bulb);
}

int wiz_bulb_turn_on(wiz_bulb_t *bulb) {
  if (!bulb) return WIZ_ERR_INVALID_PARAM;

  int ret = _wiz_send_param(bulb, "setPilot", "{\"state\":true}");
  if (ret == WIZ_OK) {
    bulb->state.state = true;
  }
  return ret;
}

int wiz_bulb_turn_off(wiz_bulb_t *bulb) {
  if (!bulb) return WIZ_ERR_INVALID_PARAM;

  int ret = _wiz_send_param(bulb, "setPilot", "{\"state\":false}");
  if (ret == WIZ_OK) {
    bulb->state.state = false;
  }
  return ret;
}

int wiz_bulb_set_brightness(wiz_bulb_t *bulb, uint8_t brightness) {
  if (!bulb) return WIZ_ERR_INVALID_PARAM;

  if (brightness < 10) brightness = 10;
  if (brightness > 100) brightness = 100;

  char params[64];
  snprintf(params, sizeof(params), "{\"dimming\":%d}", brightness);

  int ret = _wiz_send_param(bulb, "setPilot", params);
  if (ret == WIZ_OK) {
    bulb->state.brightness = brightness;
    bulb->state.state = true; // setting brightness turns bulb on
  }
  return ret;
}

int wiz_bulb_set_rgb(wiz_bulb_t *bulb, uint8_t r, uint8_t g, uint8_t b) {
  if (!bulb) return WIZ_ERR_INVALID_PARAM;

  char params[128];
  snprintf(params, sizeof(params), "{\"r\":%d,\"g\":%d,\"b\":%d}", r, g, b);

  int ret = _wiz_send_param(bulb, "setPilot", params);
  if (ret == WIZ_OK) {
    bulb->state.rgb.r = r;
    bulb->state.rgb.g = g;
    bulb->state.rgb.b = b;
  }
  return ret;
}

int wiz_bulb_set_temperature(wiz_bulb_t *bulb, uint16_t temp) {
  if (!bulb) return WIZ_ERR_INVALID_PARAM;

  if (temp < WIZ_TEMP_MIN) temp = WIZ_TEMP_MIN;
  if (temp > WIZ_TEMP_MAX) temp = WIZ_TEMP_MAX;

  char params[64];
  snprintf(params, sizeof(params), "{\"temp\":%d}", temp);

  int ret = _wiz_send_param(bulb, "setPilot", params);
  if (ret == WIZ_OK) {
    bulb->state.temp = temp;
  }
  return ret;
}

int wiz_bulb_set_scene(wiz_bulb_t *bulb, uint16_t scene_id) {
  if (!bulb) return WIZ_ERR_INVALID_PARAM;

  char params[64];
  snprintf(params, sizeof(params), "{\"sceneId\":%d}", scene_id);

  int ret = _wiz_send_param(bulb, "setPilot", params);
  if (ret == WIZ_OK) {
    bulb->state.scene_id = scene_id;
  }
  return ret;
}

int wiz_bulb_update_state(wiz_bulb_t *bulb) {
  if (!bulb)
    return WIZ_ERR_INVALID_PARAM;

  char message[256];
  char response[1024];

  int ret = wiz_build_json_message(message, sizeof(message), "getPilot", NULL);
  if (ret != WIZ_OK)
    return ret;

  ret = wiz_send_receive(bulb->socket_fd, &bulb->addr, message, response,
                         sizeof(response));
  if (ret != WIZ_OK)
    return ret;

  return wiz_parse_get_pilot_response(response, &bulb->state);
}

int wiz_bulb_get_state(wiz_bulb_t *bulb, wiz_bulb_state_t *state) {
  if (!bulb || !state)
    return WIZ_ERR_INVALID_PARAM;

  memcpy(state, &bulb->state, sizeof(wiz_bulb_state_t));
  return WIZ_OK;
}

int wiz_bulb_apply_pilot(wiz_bulb_t *bulb, wiz_pilot_builder_t *builder) {
  if (!bulb || !builder)
    return WIZ_ERR_INVALID_PARAM;

  char message[512];
  char params[384];
  char response[1024];
  int offset = 0;

  offset = 1;
  params[offset - 1] = '{';

  if (builder->has_state)
    offset += snprintf(params + offset, sizeof(params) - offset,
                       "\"state\":%s,", builder->state ? "true" : "false");
  if (builder->has_brightness)
    offset += snprintf(params + offset, sizeof(params) - offset,
                       "\"dimming\":%d,", builder->brightness);
  if (builder->has_rgb)
    offset += snprintf(params + offset, sizeof(params) - offset,
                       "\"r\":%d,\"g\":%d,\"b\":%d,",
                       builder->rgb.r, builder->rgb.g, builder->rgb.b);
  if (builder->has_temp)
    offset += snprintf(params + offset, sizeof(params) - offset,
                       "\"temp\":%d,", builder->temp);
  if (builder->has_scene_id)
    offset += snprintf(params + offset, sizeof(params) - offset,
                       "\"sceneId\":%d,", builder->scene_id);
  if (builder->has_speed)
    offset += snprintf(params + offset, sizeof(params) - offset,
                       "\"speed\":%d,", builder->speed);

  if (offset > 1 && params[offset - 1] == ',')
    offset--;
  params[offset++] = '}';
  params[offset] = '\0';

  int ret =
      wiz_build_json_message(message, sizeof(message), "setPilot", params);
  if (ret != WIZ_OK)
    return ret;

  ret = wiz_send_receive(bulb->socket_fd, &bulb->addr, message, response,
                         sizeof(response));

  // update local state if successful
  if (ret == WIZ_OK) {
    if (builder->has_state)
      bulb->state.state = builder->state;
    if (builder->has_brightness)
      bulb->state.brightness = builder->brightness;
    if (builder->has_rgb)
      bulb->state.rgb = builder->rgb;
    if (builder->has_temp)
      bulb->state.temp = builder->temp;
    if (builder->has_scene_id)
      bulb->state.scene_id = builder->scene_id;
    if (builder->has_speed)
      bulb->state.speed = builder->speed;
  }

  return ret;
}
