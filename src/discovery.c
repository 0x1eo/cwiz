#include "../include/cwiz.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <ifaddrs.h>
#include <net/if.h>

static int get_local_ip_mac(char *ip_str, char *mac_str) {
  struct ifaddrs *ifaddr, *ifa;
  int found = 0;

  if (getifaddrs(&ifaddr) == -1) {
    return -1;
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;

    // skip loopback
    if (ifa->ifa_flags & IFF_LOOPBACK)
      continue;

    // IPv4
    if (ifa->ifa_addr->sa_family == AF_INET) {
      if (!found) {
        inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr,
                  ip_str, INET_ADDRSTRLEN);
        
        // mock MAC for now as getting actual MAC is platform specific and verbose
        snprintf(mac_str, 18, "001122334455"); 
        found = 1;
      }
    }
  }

  freeifaddrs(ifaddr);
  return found ? 0 : -1;
}

extern int wiz_parse_system_config(const char *json, wiz_bulb_info_t *info);

wiz_bulb_registry_t *wiz_bulb_registry_create(void) {
  wiz_bulb_registry_t *registry =
      (wiz_bulb_registry_t *)calloc(1, sizeof(wiz_bulb_registry_t));
  return registry;
}

void wiz_bulb_registry_destroy(wiz_bulb_registry_t *registry) {
  if (!registry)
    return;

  wiz_discovered_bulb_t *current = registry->bulbs;
  while (current) {
    wiz_discovered_bulb_t *next = current->next;
    free(current);
    current = next;
  }

  free(registry);
}

static void registry_add_bulb(wiz_bulb_registry_t *registry, const char *ip,
                              const char *mac) {
  // check if bulb already exists
  wiz_discovered_bulb_t *current = registry->bulbs;
  while (current) {
    if (strcmp(current->mac_address, mac) == 0) {
      return; // already registered
    }
    current = current->next;
  }

  // create new bulb entry
  wiz_discovered_bulb_t *bulb =
      (wiz_discovered_bulb_t *)malloc(sizeof(wiz_discovered_bulb_t));
  if (!bulb)
    return;

  strncpy(bulb->ip_address, ip, sizeof(bulb->ip_address) - 1);
  bulb->ip_address[sizeof(bulb->ip_address) - 1] = '\0';
  strncpy(bulb->mac_address, mac, sizeof(bulb->mac_address) - 1);
  bulb->mac_address[sizeof(bulb->mac_address) - 1] = '\0';
  bulb->next = NULL;

  // add to registry
  if (!registry->bulbs) {
    registry->bulbs = bulb;
  } else {
    current = registry->bulbs;
    while (current->next) {
      current = current->next;
    }
    current->next = bulb;
  }

  registry->count++;
}

int wiz_discover_bulbs(wiz_bulb_registry_t *registry,
                       const char *broadcast_address, int timeout) {
  if (!registry || !broadcast_address) {
    return WIZ_ERR_INVALID_PARAM;
  }

  // create UDP socket
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return WIZ_ERR_SOCKET;
  }

  // enable broadcast
  int broadcast_enable = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable,
                 sizeof(broadcast_enable)) < 0) {
    close(sock);
    return WIZ_ERR_SOCKET;
  }

  // set socket timeout
  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = 0;
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    close(sock);
    return WIZ_ERR_SOCKET;
  }

  // setup broadcast address
  struct sockaddr_in broadcast_addr;
  memset(&broadcast_addr, 0, sizeof(broadcast_addr));
  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_port = htons(WIZ_PORT);

  if (inet_pton(AF_INET, broadcast_address, &broadcast_addr.sin_addr) <= 0) {
    close(sock);
    return WIZ_ERR_INVALID_PARAM;
  }

  // prepare registration message
  char ip_str[INET_ADDRSTRLEN] = "0.0.0.0";
  char mac_str[18] = "000000000000";
  char msg[512];
  
  get_local_ip_mac(ip_str, mac_str);
  
  snprintf(msg, sizeof(msg), 
    "{\"method\":\"registration\",\"params\":{\"phoneMac\":\"%s\",\"register\":false,\"phoneIp\":\"%s\",\"id\":\"1\"}}",
    mac_str, ip_str);

  // send broadcast message
  ssize_t sent =
      sendto(sock, msg, strlen(msg), 0,
             (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));

  if (sent < 0) {
    close(sock);
    return WIZ_ERR_SOCKET;
  }

  // receive responses
  time_t start_time = time(NULL);
  char response[2048];

  while (time(NULL) - start_time < timeout) {
    struct sockaddr_in from_addr;
    socklen_t addr_len = sizeof(from_addr);

    ssize_t received = recvfrom(sock, response, sizeof(response) - 1, 0,
                                (struct sockaddr *)&from_addr, &addr_len);

    if (received > 0) {
      response[received] = '\0';

      // extract IP address
      char ip_str[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &from_addr.sin_addr, ip_str, sizeof(ip_str));

      // parse MAC address from response
      const char *mac_ptr = strstr(response, "\"mac\":");
      if (mac_ptr) {
        mac_ptr += 7; // skip "mac":" "
        while (*mac_ptr == ' ' || *mac_ptr == '\"')
          mac_ptr++;

        char mac_str[18];
        sscanf(mac_ptr, "%17[^\"]", mac_str);

        // add to registry
        registry_add_bulb(registry, ip_str, mac_str);
      }
    } else if (received < 0 && errno == EAGAIN) {
      // timeout - no more responses
      break;
    }
  }

  close(sock);
  return registry->count;
}

wiz_discovered_bulb_t *wiz_registry_get_by_mac(wiz_bulb_registry_t *registry,
                                               const char *mac_address) {
  if (!registry || !mac_address) {
    return NULL;
  }

  wiz_discovered_bulb_t *current = registry->bulbs;
  while (current) {
    if (strcmp(current->mac_address, mac_address) == 0) {
      return current;
    }
    current = current->next;
  }

  return NULL;
}
