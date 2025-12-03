// Discovers all WiZ bulbs on the local network using UDP broadcast.
// Currently non-functional.

#include "cwiz.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  const char *broadcast_addr = "255.255.255.255";
  int timeout = 5;

  // allow custom broadcast address
  if (argc > 1) {
    broadcast_addr = argv[1];
  }

  // allow custom timeout
  if (argc > 2) {
    timeout = atoi(argv[2]);
  }

  printf("cwiz Example - Bulb Discovery\n");
  printf("==================================\n\n");

  printf("Searching for WiZ bulbs on network...\n");
  printf("Broadcast address: %s\n", broadcast_addr);
  printf("Timeout: %d seconds\n\n", timeout);

  // create registry
  wiz_bulb_registry_t *registry = wiz_bulb_registry_create();
  if (!registry) {
    fprintf(stderr, "Error: Failed to create bulb registry\n");
    return 1;
  }

  // discover bulbs
  int count = wiz_discover_bulbs(registry, broadcast_addr, timeout);

  if (count < 0) {
    fprintf(stderr, "Error during discovery: %s\n", wiz_strerror(count));
    wiz_bulb_registry_destroy(registry);
    return 1;
  }

  printf("Found %d bulb(s):\n\n", count);

  // Print discovered bulbs
  wiz_discovered_bulb_t *bulb = registry->bulbs;
  int index = 1;

  while (bulb) {
    printf("Bulb #%d:\n", index);
    printf("  IP Address:  %s\n", bulb->ip_address);
    printf("  MAC Address: %s\n", bulb->mac_address);
    printf("\n");

    bulb = bulb->next;
    index++;
  }

  // clean up
  wiz_bulb_registry_destroy(registry);

  return 0;
}
