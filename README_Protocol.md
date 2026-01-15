REPL Protocol
===============

When started, Espruino provides a REPL on whatever serial device is available (UART, Serial, Bluetooth, USB, etc).

The terminal works like a normal VT100 terminal, and displays `>` as a prompts and `:` for subsequent lines:

* ASCII characters add to the line
* Characters and arrow keys are echoed back to the user (unless `echo(0)`)
* Char code 8 is backspace
* Newlines either:
  * Execute the command if there are no unmatched braces eg `hello(` rather than `hello()`
  * Start a new line
* ASCII CSI Sequences are used for arrow keys (https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_(Control_Sequence_Introducer)_sequences) eg `ESC [ A` for up
* UTF8 codes are not understood by the terminal, but are treated as normal characters

There are other codes added though:

* 1 - SOH, packet transfer start if preceeded by DLE, or Ctrl-A clear line
* 2 - Ctrl-C - BREAKS OUT OF RUNNING CODE OR CLEARS INPUT LINE IF NONEMPTY
* 4 - Ctrl-d - backwards delete
* 5 - Ctrl-e - end of line (or on a new line, ENQ(enquiry) outputs `Espruino 2v25 JOLTJS\n` or similar
* 16 - DLE - echo off if at beginning of line
* 21 - Ctrl-u - delete line
* 23 - Ctrl-w - delete word (currently just does the same as Ctrl-u)

## Packet Transfer

Added in 2v25. There is currently an implementation of this in https://github.com/espruino/EspruinoWebTools/blob/master/uart.js
Compression added in 2v29


### Control Characters

Packet transfer is initiated by sending the `DOH`[ASCII 16] (Data Link Escape) control character, followed by the `SOH`[ASCII 1] (Start of Heading) control character.

### Heading Data

Following the initial control characters, the next two bytes signal the type and size of packet being sent. The first 3 bits contain the packet type, the following 13 represent an unsigned integer containing the byte length of the data that will follow.

| Packet Type   | Hex Representation | 16 bit Binary Representation | Notes                                                                                                       |
| ------------- | ------------------ | ---------------------------- | ----------------------------------------------------------------------------------------------------------- |
| `RESPONSE`    | `0x0000`           | `000-------------`           | Response to an `EVAL` packet                                                                                |
| `EVAL`        | `0x2000`           | `001-------------`           | Execute and return the result as `RESPONSE` packet                                                          |
| `EVENT`       | `0x4000`           | `010-------------`           | Parse data as JSON and create `E.on('packet', ...)` event                                                   |
| `FILE_SEND`   | `0x6000`           | `011-------------`           | Called before `DATA`, with `{fn:"filename",s:123}`                                                          |
| `DATA`        | `0x8000`           | `100-------------`           | Sent after `FILE_SEND` with blocks of data for the file                                                     |
| `FILE_RECV`   | `0xA000`           | `101-------------`           | Receive a file - returns a series of `DATA` packets, with a final zero length packet to end (calling data?) |
| Length (low)  | `0x0001`           | `---0000000000001`           | Minimum packet data length, 1 byte                                                                          |
| Length (high) | `0x1FFF`           | `---1111111111111`           | Maximum packet data length, 8191 bytes                                                                      |

### Responses

Following transmission of a packet (or after timeout of 1 second), there will be one of two responses that signal successful or unsuccessful packet transmission.

| Type   | ASCII character | Notes                                   |
| ------ | --------------- | --------------------------------------- |
| `ACK`  | 6               | Acknowledged successful receipt.        |
| `NACK` | 21              | Negative acknowledgement, unsuccessful. |

### Timeouts

* If a single packet takes more than 2 sec to transmit (1 sec on 2v25) Espruino will send `NAK` and will return to normal REPL mode.
* If there's more than a 10 sec gap between DATA transfers for a file, Espruino will automatically close the transfer and the file. Subsequent DATA packets will be `NAK`ed as no file is open.

### FILE_SEND packet

This is arbitrary RJSON (no quotes needed on field names). It can include:

```
{
  fn : "", // filename
  s : 123, // file size
  fs : 0/1, // if 1 (and supported) store in the FAT filesystem
  c : 0/1, // if 1 (and 2v29+) send packets heatshrink compressed - each packet is compressed in its own right
}
```

### Example

As an example, to create a file named `hello.txt` with the content `Hello World`, the packet is constructed as follows:

| Bytes                                                                        |                                                                                                                                                                                                  |
| ---------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `10 01`                                                                      | DOH, SOH, control characters                                                                                                                                                                     |
| `60 19`                                                                      | The `FILE_SEND` packet type is sent initially, and this initial packet will be 19 bytes.                                                                                                         |
| `7b 22 66 6e 22 3a 22 68 65 6c 6c 6f 2e 74 78 74 22 2c 22 73 22 3a 31 31 7d` | The data that follows is the 19 bytes that contain the JSON specifying which file to create, and that the file will be 11 bytes (the length of "Hello World") <br /> `{"fn":"hello.txt","s":11}` |
| `10 01`                                                                      | DOH, SOH, to start another packet transfer                                                                                                                                                       |
| `80 11`                                                                      | Signals this time that the packet type will be `DATA` and it will be 11 bytes.                                                                                                                   |
| `48 65 6c 6c 6f 20 57 6f 72 6c 64`                                           | The 11 bytes of data that contain the string `Hello World`                                                                                                                                       |
