# EmbedKit_UART_Frame_Parser

## Overview

This assigment implements a UART Frame Parser in C using a finite state machine approach. The parser processes incoming UART data one byte at a time and reconstructs protocol frames based on the specified format.

The implementation was developed as part of the Embedded Developer Assessment and follows the requirements specified in the problem statement.

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

The parser processes one byte at a time and transitions between states based on the received data.

### Inter-Byte Timeout Handling

- Configurable timeout value in milliseconds.
- Timeout check is performed before processing the incoming byte.
- Parser resets automatically when the gap between consecutive bytes exceeds the configured timeout.
- Timeout can be disabled by setting timeout value to 0.

### Checksum Verification

- Checksum is calculated using XOR operation.
- Received checksum is compared with calculated checksum.
- Valid frames are accepted.
- Invalid frames generate checksum error and parser reset.

### Recovery Mechanism

- Automatic recovery after timeout.
- Re-feeding of the current byte after timeout reset.
- Ability to resynchronize with the next valid Start Of Frame.

### Frame Validation

The parser supports:

- Valid frame detection
- Checksum error detection
- Timeout detection
- Back-to-back frame processing
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

Frame:

AA 01 03 10 20 30 02

Expected Result:

- Frame parsed successfully.
- Checksum validated.
- FRAME OK generated.

### Test Case 2 – Timeout Mid-Frame and Recovery

Scenario:

- Timeout occurs after receiving partial frame.
- Parser resets automatically.
- Same byte is re-fed.
- Parser synchronizes with next valid frame.

Recovery Frame:

AA 05 01 7F 7B

Checksum:

0x05 ^ 0x01 ^ 0x7F = 0x7B

### Test Case 3 – Two Valid Frames Back-to-Back

Frame 1:

AA 03 01 55 57

Checksum:

0x03 ^ 0x01 ^ 0x55 = 0x57

Frame 2:

AA 04 02 AA BB 17

Checksum:

0x04 ^ 0x02 ^ 0xAA ^ 0xBB = 0x17

### Test Case 4 – Timeout Disabled

- Uses the same byte stream as Test Case 2.
- Timeout value set to 0.
- No timeout reset occurs.
- Frame eventually fails checksum verification as expected.

---

## Build Instructions

Compile:

gcc -Wall -std=c99 uart_parser.c -o uart_parser

Run:

./uart_parser

---

## Design Highlights

- Standard C99 implementation
- No dynamic memory allocation
- No external libraries used
- Uses fixed-width integer types from stdint.h
- Clear separation between parser logic and test harness
- Modular state machine implementation
- Readable per-byte logging output
- Clean compilation with gcc warnings enabled

---

## Note on Checksum Example

The protocol defines checksum as:

CHECKSUM = XOR(CMD, LEN, PAYLOAD)

For the frame:

AA 01 03 10 20 30

The XOR calculation evaluates to:

0x01 ^ 0x03 ^ 0x10 ^ 0x20 ^ 0x30 = 0x02

The assessment document example lists 0x22 as the checksum. The implementation follows the XOR-based protocol definition and therefore uses the mathematically correct checksum value of 0x02.
