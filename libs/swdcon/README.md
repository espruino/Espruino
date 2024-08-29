# SWDCON - console over Segger RTT

This is Espruino console over [Segger RTT](https://wiki.segger.com/RTT) technology. Can be used with [BANGLEJS2](https://www.espruino.com/Bangle.js2#hardware-swd) over charging cable or with any other device that has SWD pins available and no extra UART or USB. Also could work as initial console when porting to new devices before other drivers are working.

The console can be used via Segger RTT Viewer (if you use J-Link debugger probe) or it can be also used with OpenOCD with any debugger probe like e.g. cheap CMSIS-DAP dongle.

In future the console can be also used with any Espruino device with 2 free GPIOs acting as serial to SWD/RTT bridge.

## OpenOCD start

Attach openocd to device, for nrf52 and cmsis-dap probe it is something like `openocd -d2 -f interface/cmsis-dap.cfg  -f target/nrf52.cfg`. Then connect
to openocd interactive console via `telnet localhost 4444` (Or you can use e.g. Putty for making Telnet connection) and continue with some OpenOCD commands below:

`rtt setup 0x20000000 262144 "SEGGER RTT"` - with 256KB RAM size for nrf52840 like BANGLEJS2

`rtt setup 0x20000000 65535 "SEGGER RTT"` - with 64KB RAM size for nrf52832, newer OpenOCD may not need third "SEGGER RTT" parameter

`rtt start` - should search and find the buffer

`rtt polling_interval 50` - optional, make console I/O faster (try e.g. E.dumpVariables() with default value)

`rtt server start 2222 0` - start telnet server on port 2222 for channel 0 = SWDCON, use any free port number you wish

`rtt server start 2222 0 "\377\375\042\377\373\001"` - newer OpenOCD versions can send initial string to telnet client, this switches it to raw mode but adds some extra initial garbage also to espruino console as telnet client sends some stuff back, more info https://stackoverflow.com/questions/273261/force-telnet-client-into-character-mode

To connect to console you may use another telnet connection - `telnet localhost 2222`
By default telnet is in line mode if there is no rtt server inital string. For espruino console we need raw character mode, 
type ctrl+],Enter and then type `mode char` to switch to raw mode 
or you can use netcat https://unix.stackexchange.com/questions/767170/using-telnet-command-without-protocol-negotiation

Now press ctrl+c to clear espruino console and/or press enter few times to see some initial errors from telnet garbage

If the device is in deep sleep and nothing happens after text entry you need to wake it up to notice the input and switch console - see Known issues below.

## EspruinoTools

When the openocd rtt server is running on TCP port you can also use the `espruino` command line tool available from https://github.com/espruino/EspruinoTools  like this:

```
espruino --ide 8080 --port tcp://127.0.0.1:2222
```
Then the interactive console is available and you can also use Web IDE from web browser on http://localhost:8080
```
Espruino Command-line Tool 0.1.47
-----------------------------------

Connecting to 'tcp://127.0.0.1:2222'
Connected
Web IDE is now available on http://localhost:8080
>
```

## OpenOCD stop

`rtt stop` - stop polling data, do this also before flashing new version of espruino (server can stay running but rttt stop, setup and start is needed to run  after firmware update)

`rtt server stop 2222` - optional

`nrf52.dap dpreg 4 0 ; shutdown` - with cmsis-dap this powers down nrf52 debug hardware (`dap dpreg 4 0`) and disconnects, if you would just close openocd and nrf5x stays in debug mode it drains battery and needs reboot to clear this state

## Known issues

- clipboard paste drops data when buffer is full, this is openocd issue - it only writes first part that fits into buffer and does not retry or wait for data to go out, quick fix in branch here https://github.com/fanoush/openocd/tree/f-rtt-server-write-retry with Windows build available here https://github.com/fanoush/openocd/releases/tag/latest

- if device is in deep sleep it needs to be woken up to activate the console - press button, press enter in old console, for nrf52 also triggering TIMER1_IRQn interrupt via STIR register write in openocd works `mww 0xE000EF00 9`

## TODO

- disable/enable and allocate buffers for SDWCON dynamically at runtime

- our own SWD RTT host code instead of openocd/telnet, then any Espruino device could redirect its serial/usb/bluetooth console to SWDCON of another device, which would allow WebIDE to be used easily with target device (so e.g. Bangle.js 2 with dead bluetooth could be used with App Loader over cable)

