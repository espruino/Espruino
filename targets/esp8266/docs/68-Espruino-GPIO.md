GPIO is the ability to drive IO through commands.  The functions that relate to GPIO are:

* digitalRead
* digitalWrite
* digitalPulse
* getPinMode
* pinMode

From an Espruino implementation perspective, the contract is implemented in the ESP8266 specific jshardware.c source file.  We are responsible
for implementing the following functions

    void jshPinSetState(Pin pin, JshPinState state)
    JshPinState jshPinGetSate(Pin pin)
    void jshPinSetValue(Pin pin, bool value)
    bool jshPinGetValue(Pin pin)
    JsVarFloar jshPinAnalog(Pin pin)
    int jshPinAnalogFast(Pin pin)
    JshPinFunction jshPinAnalogOutput(Pin pin,
       JsVarFloat value,
       JsVarFloat freq,
       JshAnalogOutputFlags flags)
    void jshPinPulse(Pin pin, bool value, JsVarFloat time)
    bool jshCanWatch(Pin pin)
    IOEventFlags jshPinWatch(Pin pin, bool shouldWatch)
    JshPinFunction jshGetCurrentPinFunction(Pin pin)
    bool jshIsEventForPin(IOEvent *event, Pin pin)
    IOEventFlags pinToEVEXTI(Pin pin)

The `Pin` data type is simply an `unsigned char`.

There is also a Pin Class.  This has methods for:

* constructor
* getMode
* mode
* read
* reset - Set the pin to a 0
* set - Set the pin to a 1
* write
* writeAtTime

For example, to create a definition for a Pin we might code:

    var myGPIO0 = new Pin(0);

To write a value we might code:

    myGPIO0.write(true);

or

    digitalWrite(myGPIO0, true);

The JshPinStates are:

* `JSHPINSTATE_UNDEFINED` = 0
* `JSHPINSTATE_GPIO_OUT` = 1
* `JSHPINSTATE_GPIO_OUT_OPENDRAIN` = 2
* `JSHPINSTATE_GPIO_IN` = 3
* `JSHPINSTATE_GPIO_IN_PULLUP` = 4
* `JSHPINSTATE_GPIO_IN_PULLDOWN` = 5
* `JSHPINSTATE_ADC_IN` = 6
* `JSHPINSTATE_AF_OUT` = 7
* `JSHPINSTATE_AF_OUT_OPENDRAIN` = 8
* `JSHPINSTATE_USART_IN` = 9
* `JSHPINSTATE_USART_OUT` = 10
* `JSHPINSTATE_DAC_OUT` = 11
* `JSHPINSTATE_I2C` = 12

In the build environment in the `boards` folder, there is a Python script for each of the board types.  Within this script we define a function which describes the pins available for each board.  For the ESP8266, we have a script called `ESP8266_BOARD.py` which defines the available pins.

#####devices

This is a list of built-in stuff on the board that is made accessible to Espruino. It's parsed and turned into defines `in gen/platform_config.h`.

Stuff you can use is LED1-8, BTN1-4, USB, LCD (for boards with FSMC LCDs built in), SD (SD card), JTAG (when JTAG pins are defined, but we need to make sure we leave them alone when the board resets).

A helper script is supplied by the Espruino build system called `pinutils.py`.  It contains utility functions:



----

######generate_pins
Return a set of pins from `min_pin` to `max_pin` inclusive.

    generate_pins(min_pin, max_pin)

This is achieved by calling `findpin` for each of the pins in the range with a name of `PD<Num>`.

The format returned is an array of Pin objects where each of these contains:

* `name` is the pin name - due to random historical reasons (from ST datasheets) it needs prefixing with `P`.
* `sortingname` is the name, but padded so that when it's sorted everything appears in the right order
port is the actual port - on ESP8266 this might not be needed and could just default to `D`.
* `num` is the pin number - this doesn't have to match `D` - it's what is needed internally to access the hardware. For instance Olimexino has 'logical' pins that actually map all over the place.
* `function` is a map of pin functions to their 'alternate functions' (an STM32 chip thing - STM32F4 chips can have different peripherals on each pin, so the alternate function is a number that you shove in that pin's register in order to connect it to that peripheral). The format, for instance I2C1_SDA is important as it's parsed later and is used to build `gen/jspininfo.c`.
* `csv` isn't needed afaik, but when using data grabbed from csv files from ST's datasheets like this it contains the raw data for debugging)

----

######findpin
Find and add a new pin to the list of pins.  Returns the new pin.

    findpin(pins, pinname, force)
 
Find the pin definition with the name `pinname` and add it to the list of pins supplied by `pins`.  If
`force` is `true` and the named pin is not known, then an error is generated otherwise a new default pin
is created.
