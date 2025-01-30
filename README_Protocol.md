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

### Packet Transfer

Added in 2v25:

```
DLE[16],SOH[1],TYPE|LENHI,LENLO,DATA...
```

If received or timed out (after 1s), will reply with an ACK[6] or NAK[21]

TYPE is:

```
PT_TYPE_RESPONSE = 0x0000, // Response to an EVAL packet
PT_TYPE_EVAL = 0x2000,  // execute and return the result as RESPONSE packet
PT_TYPE_EVENT = 0x4000, // parse as JSON and create `E.on('packet', ...)` event
PT_TYPE_FILE_SEND = 0x6000, // called before DATA, with {fn:"filename",s:123}
PT_TYPE_DATA = 0x8000, // Sent after FILE_SEND with blocks of data for the file
PT_TYPE_FILE_RECV = 0xA000 // receive a file - returns a series of PT_TYPE_DATA packets, with a final zero length packet to end
```

There is currently an implementation of this in https://github.com/espruino/EspruinoWebTools/blob/master/uart.js
