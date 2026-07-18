# NetSpyre

NetSpyre is a C-based Linux networking inspection tool built on raw Netlink (`NETLINK_ROUTE`) sockets.
It queries kernel networking state and prints information for common RTNETLINK object types:

- `link`  → network interfaces
- `addr`  → IP addresses
- `route` → routing table entries
- `neigh` → neighbor/ARP table entries

## Features

- Uses low-level Netlink messaging (`sendto`/`recv`) directly in C
- Separate modules for each object family (`link`, `addr`, `route`, `neigh`)
- Simple CLI command dispatch from `main.c`
- Reusable Netlink send/receive helper layer in `src/netlink.c`

## Repository Layout

- `src/main.c` — CLI entry point and command dispatch
- `src/netlink.c` — generic Netlink request/receive helpers
- `src/link.c` — link request/parse logic
- `src/addr.c` — address request/parse logic
- `src/route.c` — route request/parse logic
- `src/neigh.c` — neighbor request/parse logic
- `include/*.h` — module interfaces and shared declarations
- `netlink_test` — binary/artifact currently present in repo root

## Requirements

- Linux (RTNETLINK is Linux-specific)
- C compiler (e.g., `gcc`)
- Sudo/root privileges for raw Netlink socket operations

## Build

Example build command from repository root:

```bash
gcc -Wall -Wextra -O2 -Iinclude src/*.c -o netspyre
```

## Usage

```bash
sudo ./netspyre [link | addr | route | neigh]
```

Examples:

```bash
sudo ./netspyre link
sudo ./netspyre addr
sudo ./netspyre route
sudo ./netspyre neigh
```

## How it works (high level)

1. `main.c` opens a raw Netlink socket with protocol `NETLINK_ROUTE`.
2. Socket is bound to the calling process PID.
3. Selected command sends an RTNETLINK request (`RTM_GET*`) via helper functions.
4. Response messages are received and validated (`NLMSG_OK`, `NLMSG_ERROR`).
5. Module-specific parsers print decoded kernel networking data.

## Notes

- Current CLI commands are strict and positional: one command argument is required.
- This project is focused on inspection/query behavior (not configuration changes).
- Spelling in some function names (for example `recieve_*`) follows current source as-is.

## License

No license file is currently present in this repository.
If you plan to publish or reuse this code, consider adding a `LICENSE` file.
