# SWDCON - console over Segger RTT

This is Espruino console over [Segger RTT](https://wiki.segger.com/RTT) technology. Can be used with [BANGLEJS2](https://www.espruino.com/Bangle.js2#hardware-swd) over charging cable or with any other device that has SWD pins available and no extra UART or USB. Also could work as initial console when porting to new devices before other drivers are working.

The console can be used via Segger RTT Viewer (if you use J-Link debugger probe) or it can be also used with OpenOCD with any debugger probe like e.g. cheap CMSIS-DAP dongle.

Another way instead of running OpenOCD is dapjs over WebUSB supported in Chrome browser.

In future the console can be also used with any Espruino device with 2 free GPIOs acting as serial to SWD/RTT bridge.

## dapjs over WebUSB

There is a project that allows using CMSIS-DAP debug probe directly from web browser https://armmbed.github.io/dapjs/docs/index.html

 You can try customized example here https://fanoush.github.io/dapjs/examples/rtt/web.html

 Best is to enable character mode, then connect to your probe via 'start RTT', then press enter and then click the 'trigger IRQ' button and you should get console output - see also Known issues below.

 Unfortunately only newer CMSIS-DAP probes that support V2 protocol work over WebUSB in browser, older v1 probes that work over HID protocol do not work. If your probe is not working the easiest is to get Raspberry Pico or any other RP2040 board and flash it with debugprobe firmware https://github.com/raspberrypi/debugprobe which does support V2 protocol

## OpenOCD

OpenOCD gives best compatibility with many debug probes.
Attach openocd to device, for nrf52 and cmsis-dap probe it is something like `openocd -d2 -f interface/cmsis-dap.cfg  -f target/nrf52.cfg`. Then connect
to openocd interactive console via `telnet localhost 4444` (Or you can use e.g. Putty for making Telnet connection) and continue with some OpenOCD commands below:

`rtt setup 0x20000000 262144 "SEGGER RTT"` - with 256KB RAM size for nrf52840 like BANGLEJS2

`rtt setup 0x20000000 65535 "SEGGER RTT"` - with 64KB RAM size for nrf52832, newer OpenOCD may not need third "SEGGER RTT" parameter

`rtt start` - should search and find the buffer

`rtt polling_interval 30` - optional, make console I/O faster (try to run e.g. `E.dumpVariables()` with default value)

`rtt server start 9090 0` - start telnet server on port 9090 for channel 0 = SWDCON, use any free port number you wish, 9090 is typical for RTT channel 0

You can also run all commands directly when starting OpenOCD

`openocd -d2  -f interface/cmsis-dap.cfg  -f target/nrf52.cfg -c "adapter speed 8000" -c "init" -c "rtt setup 0x20000000 262144" -c "rtt start ; rtt server start 9090 0" -c "rtt polling_interval 30"`

Many good probes including RP2040 debugprobe support higher adapter speeds, for nrf52 chips maximum supported is 8MHz, if you have issues skip `-c "adapter speed 8000"` or go lower to 4 or 2 MHz


## EspruinoTools

When the openocd rtt server is running on TCP port you can use the `espruino` command line tool available from https://github.com/espruino/EspruinoTools  like this:

```
espruino --ide 8080 --port tcp://127.0.0.1:9090
```
Then the interactive console is available and you can also use Web IDE from web browser on http://localhost:8080
```
Espruino Command-line Tool 0.1.47
-----------------------------------

Connecting to 'tcp://127.0.0.1:9090'
Connected
Web IDE is now available on http://localhost:8080
>
```

Beware that by default device is in deep sleep and nothing happens after first text entry, you need to wake it up to notice the input and switch to RTT console - see Known issues below.


## telnet client

While espruino command line tool is easier, you may also use telnet client directly via  `telnet localhost 9090`

By default telnet is in line mode if there is no rtt server inital string. For espruino console we need raw character mode, 
type ctrl+],Enter and then type `mode char` to switch to raw mode 
or you can use netcat https://unix.stackexchange.com/questions/767170/using-telnet-command-without-protocol-negotiation

Now press ctrl+c to clear espruino console and/or press enter few times to see some initial errors from telnet garbage

`rtt server start 9090 0 "\377\375\042\377\373\001"` - newer OpenOCD versions can send initial string to telnet client, this switches it to raw mode automatically but adds some extra initial garbage also to Espruino console as telnet client sends some stuff back, more info https://stackoverflow.com/questions/273261/force-telnet-client-into-character-mode


## OpenOCD stop

`rtt stop` - stop polling data, do this also before flashing new version of espruino (server can stay running but rttt stop, setup and start is needed to run  after firmware update)

`rtt server stop 9090` - optional

`nrf52.dap dpreg 4 0 ; shutdown` - with cmsis-dap this powers down nrf52 debug hardware (`dap dpreg 4 0`) and disconnects, if you would just close/kill openocd and nrf5x stays in debug mode it drains battery and needs reboot to clear this state

## Known issues

- clipboard paste drops data when buffer is full, this is [openocd issue](https://review.openocd.org/c/openocd/+/8360) - it only writes first part that fits into buffer and does not retry or wait for data to go out, quick fix in branch here https://github.com/fanoush/openocd/tree/f-rtt-server-write-retry with Windows build available here https://github.com/fanoush/openocd/releases

- if device is in deep sleep it needs to be woken up to activate the console - press button, press enter in old console, for nrf52 also triggering TIMER1_IRQn interrupt via STIR register write in openocd works `mww 0xE000EF00 9`

## TODO

- disable/enable and allocate buffers for SWDCON dynamically at runtime

- our own SWD RTT host code instead of openocd/telnet, then any Espruino device could redirect its serial/usb/bluetooth console to SWDCON of another device, which would allow WebIDE to be used easily with target device (so e.g. Bangle.js 2 with dead bluetooth could be used with App Loader over cable)

