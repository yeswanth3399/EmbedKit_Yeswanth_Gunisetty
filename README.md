# EmbedKit_Yeswanth

## Author

**Yeswanth Gunisetty**

---

## Module Summary

| Module | Description |
|----------|-------------|
| uart_parser.c | UART frame parser implemented using a finite state machine with checksum validation, inter-byte timeout detection, timeout recovery, and frame verification. |

---

## Build Instructions

Compile:

```bash
gcc -Wall -std=c99 uart_parser.c -o uart_parser
```

Run:

```bash
./uart_parser
```

---

## Repository Structure

```text
EmbedKit_Yeswanth/
│
├── uart_parser.c
├── README.md
└── .gitignore
```

---

## Overview

This assignment implements a UART Frame Parser in C using a finite state machine approach. The parser processes incoming UART data one byte at a time and reconstructs protocol frames based on the specified format.

The implementation follows the requirements provided in the Embedded Developer Assessment.

---

## Protocol Format

Frame Structure:

SOF | CMD | LEN | PAYLOAD | CHECKSUM

- SOF (Start of Frame): 0xAA
- CMD: Command Identifier
- LEN: Number of payload bytes (0–16)
- PAYLOAD: LEN bytes of data
- CHECKSUM: XOR of CMD, LEN and all payload bytes

Checksum Formula:

CHECKSUM = CMD ^ LEN ^ PAYLOAD[0] ^ PAYLOAD[1] ...

---

## Features Implemented

### State Machine Based Parser

The parser is implemented using the following states:

- UART_STATE_WAIT_SOF
- UART_STATE_CMD
- UART_STATE_LEN
- UART_STATE_PAYLOAD
- UART_STATE_CHECKSUM

### Inter-Byte Timeout Handling

- Configurable timeout value in milliseconds
- Timeout check performed before processing the current byte
- Automatic parser reset after timeout
- Timeout disable support using timeout value 0

### Checksum Verification

- XOR-based checksum calculation
- Checksum validation for received frames
- Checksum error detection and parser recovery

### Recovery Mechanism

- Automatic timeout recovery
- Re-feeding of the current byte after timeout reset
- Re-synchronization using the next valid SOF byte

### Frame Validation

- Valid frame detection
- Checksum error detection
- Timeout detection
- Back-to-back frame support
- Payload length validation

---

## Return Codes

| Return Value | Description |
|-------------|-------------|
| 1 | Valid frame received |
| 0 | Frame still in progress |
| -1 | Checksum mismatch |
| -2 | Inter-byte timeout occurred |

---

## Test Cases Implemented

### Test Case 1 – Clean Valid Frame

- Valid frame reception
- Checksum verification
- Successful frame decoding

### Test Case 2 – Timeout Mid-Frame and Recovery

- Timeout detection after partial frame reception
- Parser reset
- Re-feed mechanism validation
- Recovery frame parsing

### Test Case 3 – Two Valid Frames Back-to-Back

- Consecutive frame processing
- State machine reset verification
- Multiple valid frame handling

### Test Case 4 – Timeout Disabled

- Timeout checking disabled
- Verification of checksum error behavior without timeout recovery

---

## Design Highlights

- Standard C99 implementation
- No dynamic memory allocation
- No external libraries
- Fixed-width integer types from stdint.h
- Byte-by-byte frame processing
- Modular state machine design
- Readable logging output
- Clean compilation using gcc warnings

---

## Note on Checksum Example

The protocol defines checksum as:

CHECKSUM = XOR(CMD, LEN, PAYLOAD)

For the frame:

AA 01 03 10 20 30

The XOR calculation evaluates to:

0x01 ^ 0x03 ^ 0x10 ^ 0x20 ^ 0x30 = 0x02

The implementation follows the XOR-based protocol definition.
