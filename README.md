# cwiz

**A lightweight, dependency-free C library for WiZ Connected smart lights.**

`cwiz` provides a direct interface to WiZ bulbs, strips, and sockets via raw UDP. It is a C implementation of the logic found in [pywizlight](https://github.com/sbidy/pywizlight), designed for embedded environments, system daemons, or anyone who prefers manual memory management over a Python runtime.

It handles the JSON protocol, discovery, and state management without the overhead.

## Capabilities

  * **Core Control:** Toggle power, set brightness, RGB, and color temperature (Kelvin).
  * **Scene Management:** Native support for WiZ pre-sets (Ocean, Sunset, Pulse, etc.).
  * **Discovery:** Broadcast-based detection of devices on the local subnet.
  * **Pilot Builder:** Construct complex state changes (e.g., color + brightness + state) and commit them in a single packet.
  * **Zero Bloat:** Depends only on standard C libraries and POSIX sockets.

## Build & Install

Standard Makefile workflow. No configure scripts, no external package managers.

```bash
# Build library and examples
make

# Install headers and shared library (default: /usr/local)
sudo make install

# Clean build artifacts
make clean
```

## Usage

### 1\. Basic Control

The API is synchronous and blocking. This is a design choice for simplicity in sequential logic.

```c
#include <cwiz.h>

int main() {
    // Initialize connection
    wiz_bulb_t *bulb = wiz_bulb_create("192.168.1.100");
    if (!bulb) return 1;

    // Operations
    wiz_bulb_turn_on(bulb);
    wiz_bulb_set_brightness(bulb, 50);      // 0-100
    wiz_bulb_set_rgb(bulb, 0, 0, 255);       // Blue
    wiz_bulb_set_temperature(bulb, 4000);    // 4000K

    // Cleanup
    wiz_bulb_destroy(bulb);
    return 0;
}
```

### 2\. Network Discovery

Scans the subnet via UDP broadcast.

```c
#include <cwiz.h>
#include <stdio.h>

int main() {
    wiz_bulb_registry_t *registry = wiz_bulb_registry_create();
    
    // Broadcast to 255.255.255.255 with 5s timeout
    int count = wiz_discover_bulbs(registry, "255.255.255.255", 5);
    printf("Discovered %d devices.\n", count);
    
    // Linked list iteration
    wiz_discovered_bulb_t *node = registry->bulbs;
    while (node) {
        printf("[%s] %s\n", node->mac_address, node->ip_address);
        node = node->next;
    }
    
    wiz_bulb_registry_destroy(registry);
    return 0;
}
```

### 3\. The Pilot Builder

For atomic updates, use the Pilot Builder. This prevents the "popcorn effect" where brightness applies before color.

```c
wiz_pilot_builder_t *pb = wiz_pilot_builder_create();

wiz_pilot_builder_set_state(pb, true);
wiz_pilot_builder_set_scene(pb, 12); // Ocean
wiz_pilot_builder_set_brightness(pb, 100);

// Commit all changes in one UDP packet
wiz_bulb_apply_pilot(bulb, pb);

wiz_pilot_builder_destroy(pb);
```

## Examples

Four complete programs in `examples/` show how to use the library:

```bash
./build/discovery
# Scans your network and lists all WiZ devices with their IPs and MAC addresses
# This currently only works sometimes (at least in my current environment).

./build/basic_control 192.168.1.100
# Cycles through colors (red/green/blue), color temps (warm/cool white),
# queries bulb state, then turns it off

./build/scenes 192.168.1.100
# Displays all 37 built-in scenes with their IDs and applies each one briefly

./build/pilot_builder 192.168.1.100
# Demonstrates atomic updates—sets color, brightness, and state in one packet
```

All examples include proper error handling. Check the source in `examples/` to see how to handle timeouts, parse responses, and recover from failures.


## Rationale & Architecture

**Why C?**
Python is excellent, but heavy. `cwiz` allows integration into low-resource environments (routers, IoT bridges) or larger C/C++ projects where spawning a Python interpreter is unacceptable.

**Protocol**
WiZ lights communicate via UDP on port `38899`. Messages are JSON-formatted.

  * **Request:** `{"method":"setPilot","params":{"r":255,"g":0,"b":0}}`
  * **Response:** `{"method":"setPilot","result":{"success":true}}`

`cwiz` handles the socket creation, JSON serialization (custom, lightweight implementation), and response parsing.

**Sync vs Async**
Calls in `cwiz` block until a response is received or a timeout occurs. If you need asynchronous control, wrap `cwiz` calls in your own event loop or thread pool.

## Error Handling

All state-modifying functions return an integer status code.

| Code | Macro | Description |
| :--- | :--- | :--- |
| `0` | `WIZ_OK` | Success. |
| `-1` | `WIZ_ERR_SOCKET` | Socket creation/write failure. |
| `-2` | `WIZ_ERR_TIMEOUT` | No response within defined window. |
| `-4` | `WIZ_ERR_JSON_PARSE` | Malformed response from device. |
| `-7` | `WIZ_ERR_CONNECTION` | Unreachable destination. |

Use `wiz_strerror(code)` for a string representation.

## License & Attribution

**MIT License**

Copyright (c) 2025 Leandro Afonso (0x1eo)

This project is a derivative work based on **pywizlight** by Stephan Traub (sbidy).
The logic for scene mapping and protocol handshake was adapted from the original Python implementation.

  * **Original Project:** [pywizlight](https://github.com/sbidy/pywizlight)
  * **Full License:** See `LICENSE` file for the complete stacked copyright notices.