#ifndef CWIZ_H
#define CWIZ_H

#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>

#define CWIZ_VERSION "1.0.0"
#define WIZ_PORT 38899
#define WIZ_DEFAULT_TIMEOUT 13
#define WIZ_MAX_RETRIES 6
#define WIZ_TEMP_MIN 2200
#define WIZ_TEMP_MAX 6500

// error codes
typedef enum {
  WIZ_OK = 0,
  WIZ_ERR_SOCKET = -1,
  WIZ_ERR_TIMEOUT = -2,
  WIZ_ERR_INVALID_PARAM = -3,
  WIZ_ERR_JSON_PARSE = -4,
  WIZ_ERR_NO_RESPONSE = -5,
  WIZ_ERR_MALLOC = -6,
  WIZ_ERR_CONNECTION = -7
} wiz_error_t;

typedef struct wiz_bulb wiz_bulb_t;
typedef struct wiz_pilot_builder wiz_pilot_builder_t;
typedef struct wiz_discovered_bulb wiz_discovered_bulb_t;
typedef struct wiz_bulb_registry wiz_bulb_registry_t;

// color representations
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} wiz_rgb_t;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t c; // cool white
  uint8_t w; // warm white
} wiz_rgbcw_t;

// state
typedef struct {
  bool state;         // on/off
  uint8_t brightness; // 10-100
  uint16_t scene_id;  // scene ID
  uint8_t speed;      // speed for scenes
  uint16_t temp;      // color temperature (Kelvin)
  wiz_rgb_t rgb;      // RGB color
  wiz_rgbcw_t rgbcw;  // RGBCW color
  char src[32];       // source of state change
  int rssi;           // wifi signal strength
} wiz_bulb_state_t;

// info
typedef struct {
  char mac_address[18];
  char module_name[64];
  char firmware_version[32];
  char home_id[64];
  char room_id[64];
} wiz_bulb_info_t;

// scenes
typedef struct {
  uint16_t id;
  const char *name;
} wiz_scene_t;

struct wiz_discovered_bulb {
  char ip_address[16];
  char mac_address[18];
  struct wiz_discovered_bulb *next;
};

// registry
struct wiz_bulb_registry {
  wiz_discovered_bulb_t *bulbs;
  int count;
};

// main structure
struct wiz_bulb {
  char ip_address[16];
  int port;
  int socket_fd;
  struct sockaddr_in addr;
  wiz_bulb_state_t state;
  wiz_bulb_info_t info;
};

struct wiz_pilot_builder {
  bool has_state;
  bool state;
  bool has_brightness;
  uint8_t brightness;
  bool has_rgb;
  wiz_rgb_t rgb;
  bool has_temp;
  uint16_t temp;
  bool has_scene_id;
  uint16_t scene_id;
  bool has_speed;
  uint8_t speed;
};

// bulb control functions
wiz_bulb_t *wiz_bulb_create(const char *ip_address);
void wiz_bulb_destroy(wiz_bulb_t *bulb);
int wiz_bulb_turn_on(wiz_bulb_t *bulb);
int wiz_bulb_turn_off(wiz_bulb_t *bulb);
int wiz_bulb_set_brightness(wiz_bulb_t *bulb, uint8_t brightness);
int wiz_bulb_set_rgb(wiz_bulb_t *bulb, uint8_t r, uint8_t g, uint8_t b);
int wiz_bulb_set_temperature(wiz_bulb_t *bulb, uint16_t temp);
int wiz_bulb_set_scene(wiz_bulb_t *bulb, uint16_t scene_id);
int wiz_bulb_update_state(wiz_bulb_t *bulb);
int wiz_bulb_get_state(wiz_bulb_t *bulb, wiz_bulb_state_t *state);
int wiz_bulb_apply_pilot(wiz_bulb_t *bulb, wiz_pilot_builder_t *builder);

// pilot builder functions
wiz_pilot_builder_t *wiz_pilot_builder_create(void);
void wiz_pilot_builder_destroy(wiz_pilot_builder_t *builder);
void wiz_pilot_builder_set_state(wiz_pilot_builder_t *builder, bool state);
void wiz_pilot_builder_set_brightness(wiz_pilot_builder_t *builder,
                                      uint8_t brightness);
void wiz_pilot_builder_set_rgb(wiz_pilot_builder_t *builder, uint8_t r,
                               uint8_t g, uint8_t b);
void wiz_pilot_builder_set_temperature(wiz_pilot_builder_t *builder,
                                       uint16_t temp);
void wiz_pilot_builder_set_scene(wiz_pilot_builder_t *builder,
                                 uint16_t scene_id);

// discovery and registry functions
wiz_bulb_registry_t *wiz_bulb_registry_create(void);
void wiz_bulb_registry_destroy(wiz_bulb_registry_t *registry);
int wiz_discover_bulbs(wiz_bulb_registry_t *registry,
                       const char *broadcast_address, int timeout);

wiz_discovered_bulb_t *wiz_registry_get_by_mac(wiz_bulb_registry_t *registry,
                                               const char *mac_address);

// scene functions
const char *wiz_get_scene_name(uint16_t scene_id);
uint16_t wiz_get_scene_id(const char *scene_name);
const wiz_scene_t *wiz_get_all_scenes(int *count);

// utility functions
const char *wiz_strerror(int error);
void wiz_rgb_to_rgbcw(wiz_rgb_t rgb, uint16_t temp, wiz_rgbcw_t *rgbcw);
int wiz_hex_to_rgb(const char *hex_color, wiz_rgb_t *rgb);

#endif // CWIZ_H
