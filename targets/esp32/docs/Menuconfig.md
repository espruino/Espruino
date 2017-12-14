# make menuconfig
Within the template project that builds the ESP-IDF environment, there is a
configuration file called `sdkconfig`.  While this file can be edited by hand
there is an elegant full screen text based user interface for editing.  We can
bring this up by running `make menuconfig`.  This provides a menu driven editor
where properties can be changed.

Some options *must* be changed for correct operation of Espruino.  These
are:

* Component config -> LWIP -> Enable SO_REUSEADDR option [Enable]
* Component config -> ESP32-specific config ->  Task watchdog [Disable]
* Component config > FreeRTOS ->  Halt when an SMP-untested function is called [Disable]

Other options can be changed at your discretion for changes in the environment that
are primarily to taste.  Examples that I would consider are:

* Bootloader config -> Bootloader log verbosity - Consider changing this to `Verbose`.
* Serial flasher config -> Default baud rate - I have had success with 921600.
* Serial flasher config -> Use compressed upload - I have had success with this enabled.
* Component config -> Log output -> Default log verbosity - Consider changing this to `Verbose`.
* Component config -> Log output -> Use ANSI terminal colors in log output - I switch this off from the default of on.

When re-importing the latest versions of ESP-IDF, we will likely overwrite any changes
we made here.  Either we copy the `sdkconfig` you have been using and put it back after
a refresh or else re-run `make menuconfig` after a refresh.