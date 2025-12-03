#include "../include/cwiz.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>



// internal helper to create socket
int wiz_create_socket(void) {
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return WIZ_ERR_SOCKET;
  }

  // set socket timeout
  struct timeval tv;
  tv.tv_sec = WIZ_DEFAULT_TIMEOUT;
  tv.tv_usec = 0;

  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    close(sock);
    return WIZ_ERR_SOCKET;
  }

  return sock;
}

// send a message and receive response
int wiz_send_receive(int sock, struct sockaddr_in *addr, const char *message,
                     char *response, size_t response_size) {
  if (!message || !response || !addr) {
    return WIZ_ERR_INVALID_PARAM;
  }

  int attempts = 0;
  // wait_time in microseconds
  unsigned int wait_time_us = 750000; // 0.75s

  while (attempts < WIZ_MAX_RETRIES) {
    // send datagram
    ssize_t sent = sendto(sock, message, strlen(message), 0,
                          (struct sockaddr *)addr, sizeof(*addr));

    if (sent < 0) {
      return WIZ_ERR_SOCKET;
    }

    // try to receive response
    struct timeval tv;
    tv.tv_sec = wait_time_us / 1000000;
    tv.tv_usec = wait_time_us % 1000000;

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    socklen_t addr_len = sizeof(*addr);
    ssize_t received = recvfrom(sock, response, response_size - 1, 0,
                                (struct sockaddr *)addr, &addr_len);

    if (received > 0) {
      response[received] = '\0';
      return WIZ_OK;
    }

    attempts++;
    wait_time_us += 3000000; // increase wait time by 3 seconds
    if (wait_time_us > (unsigned int)(WIZ_DEFAULT_TIMEOUT * 1000000)) {
      wait_time_us = (unsigned int)(WIZ_DEFAULT_TIMEOUT * 1000000);
    }
  }

  return WIZ_ERR_TIMEOUT;
}

// build JSON message
int wiz_build_json_message(char *buffer, size_t size, const char *method,
                           const char *params) {
  if (!buffer || !method) {
    return WIZ_ERR_INVALID_PARAM;
  }

  int len;
  if (params && strlen(params) > 0) {
    len = snprintf(buffer, size, "{\"method\":\"%s\",\"params\":%s}", method,
                   params);
  } else {
    len = snprintf(buffer, size, "{\"method\":\"%s\"}", method);
  }

  if (len < 0 || (size_t)len >= size) {
    return WIZ_ERR_INVALID_PARAM;
  }

  return WIZ_OK;
}

// internal helper to verify if a match is a valid key
static int _is_valid_key_start(const char *json, const char *match_ptr) {
  // if match is at the very start (unlikely for valid JSON), assume valid
  if (match_ptr == json) return 1;

  // scan backwards skipping whitespace
  const char *prev = match_ptr - 1;
  while (prev > json && (*prev == ' ' || *prev == '\t' || *prev == '\n' || *prev == '\r')) {
    prev--;
  }

  // valid key = '{' (start of object) || ',' (next item)
  return (*prev == '{' || *prev == ',');
}

// helper to extract integer value from JSON
static int _json_get_int(const char *json, const char *key) {
  char search_key[64];
  snprintf(search_key, sizeof(search_key), "\"%s\":", key);
  size_t key_len = strlen(search_key);

  const char *ptr = json;
  
  // loop through all occurrences of the key
  while ((ptr = strstr(ptr, search_key)) != NULL) {
    if (_is_valid_key_start(json, ptr)) {
      // found the real key, advance to value
      ptr += key_len;
      // Skip whitespace
      while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') {
        ptr++;
      }
      return atoi(ptr);
    }
    // false positive (key inside a string), skip and keep searching
    ptr += 1; 
  }

  return -1;
}

// helper to extract boolean value from JSON
static int _json_get_bool(const char *json, const char *key) {
  char search_key[64];
  snprintf(search_key, sizeof(search_key), "\"%s\":", key);
  size_t key_len = strlen(search_key);

  const char *ptr = json;

  while ((ptr = strstr(ptr, search_key)) != NULL) {
    if (_is_valid_key_start(json, ptr)) {
      ptr += key_len;
      while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') {
        ptr++;
      }
      return (*ptr == 't' || *ptr == 'T');
    }
    ptr += 1;
  }

  return -1;
}

// parse simple JSON response (basic parser for getPilot response)
int wiz_parse_get_pilot_response(const char *json, wiz_bulb_state_t *state) {
  if (!json || !state) {
    return WIZ_ERR_INVALID_PARAM;
  }

  int val = _json_get_bool(json, "state");
  if (val != -1) state->state = val;

  val = _json_get_int(json, "dimming");
  if (val != -1) state->brightness = val;

  val = _json_get_int(json, "temp");
  if (val != -1) state->temp = val;

  val = _json_get_int(json, "sceneId");
  if (val != -1) state->scene_id = val;

  val = _json_get_int(json, "r");
  if (val != -1) state->rgb.r = val;

  val = _json_get_int(json, "g");
  if (val != -1) state->rgb.g = val;

  val = _json_get_int(json, "b");
  if (val != -1) state->rgb.b = val;

  val = _json_get_int(json, "rssi");
  if (val != -1) state->rssi = val;

  return WIZ_OK;
}

// parse system config response
int wiz_parse_system_config(const char *json, wiz_bulb_info_t *info) {
  if (!json || !info) {
    return WIZ_ERR_INVALID_PARAM;
  }

  // parse MAC address
  const char *ptr = strstr(json, "\"mac\":");
  if (ptr) {
    ptr += 7; // skip "mac":" "
    while (*ptr == ' ' || *ptr == '\"')
      ptr++;
    sscanf(ptr, "%17[^\"]", info->mac_address);
  }

  // parse module name
  ptr = strstr(json, "\"moduleName\":");
  if (ptr) {
    ptr += 14;
    while (*ptr == ' ' || *ptr == '\"')
      ptr++;
    sscanf(ptr, "%63[^\"]", info->module_name);
  }

  // parse firmware version
  ptr = strstr(json, "\"fwVersion\":");
  if (ptr) {
    ptr += 13;
    while (*ptr == ' ' || *ptr == '\"')
      ptr++;
    sscanf(ptr, "%31[^\"]", info->firmware_version);
  }

  return WIZ_OK;
}
