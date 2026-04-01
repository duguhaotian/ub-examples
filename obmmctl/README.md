# obmmctl - OBMM Control Tool

OBMM Control Tool for memory borrowing/lending management using the Ownership-Based Memory Management (OBMM) library.

## Overview

obmmctl provides a command-line interface for managing memory regions between nodes in a distributed memory system. It supports:

- **lend**: Lend out local memory to remote nodes
- **borrow**: Borrow memory from remote nodes
- **unlend/unborrow**: Cancel memory operations
- **set-owner**: Change memory ownership
- **status**: Query memory region status

## Build

### Production Build (with real libobmm)

```bash
make
```

Requires libobmm to be installed and available.

### Test Build (with stubs)

```bash
make test
```

Uses stub implementations for unit testing without the real library.

## Commands

### lend - Lend out memory

```bash
obmmctl lend --eid <controller_path> [--size <size>] [--flags <flags>]
```

Options:
- `-e, --eid <path>`: Controller sysfs path (e.g., `/sys/devices/ub_bus_controller0/0`)
- `-s, --size <bytes>`: Size in bytes (default: 128M, supports K, M, G suffixes)
- `-f, --flags <flags>`: Export flags (default: 0x01 = ALLOW_MMAP)

Output returns `mem_id` and `addr` for use with remote borrow.

Example:
```bash
obmmctl lend --eid /sys/devices/ub_bus_controller0/0 --size 128M
```

### borrow - Borrow memory

```bash
obmmctl borrow --eid <controller_path> --addr <remote_addr> --size <size> [--flags <flags>]
```

Options:
- `-e, --eid <path>`: Local controller sysfs path
- `-a, --addr <address>`: Remote physical address (from remote lend output)
- `-s, --size <bytes>`: Size in bytes (must match remote lend size)
- `-f, --flags <flags>`: Import flags (default: 0x01 = ALLOW_MMAP)

Example:
```bash
obmmctl borrow --eid /sys/devices/ub_bus_controller0/0 --addr 0x100000000 --size 128M
```

### unlend - Cancel lend

```bash
obmmctl unlend --id <mem_id>
```

### unborrow - Cancel borrow

```bash
obmmctl unborrow --id <mem_id>
```

### set-owner - Set memory ownership

```bash
obmmctl set-owner --id <mem_id> --node <node_id>
```

### status - Show memory status

```bash
obmmctl status [--id <mem_id>]
```

### help - Show help

```bash
obmmctl help [command]
```

## Key Changes from Previous Version

1. **Simplified CLI**: Uses controller sysfs path (`--eid`) instead of raw EID values
2. **Automatic sysfs reading**: Controller parameters (EID, CNA) are read automatically from sysfs
3. **Unified logging**: Consistent log output with EID formatting support
4. **Dual build modes**: Production (real libobmm) and test (stubs)

## Memory Alignment

All memory operations require 2MB alignment. The library validates size alignment automatically.

## Log Levels

- ERROR: Critical errors
- WARN: Warnings
- INFO: Informational messages (enabled with `--verbose`)
- DEBUG: Detailed debugging info

## License

See LICENSE file in the project root.