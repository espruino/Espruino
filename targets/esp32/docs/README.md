#Notes on the development of Espruino for ESP32

The github URL for core Espruino is:

https://github.com/espruino/Espruino.git

New board will trigger on
ESP32=1

To build, we need to set
```
export ESP32=1
export ESP_IDF_PATH=/path to ESP_IDF
export PATH=/opt/xtensa-esp32-elf/bin:$PATH
```

##The Espruino Github
Espruino is maintained as Open Source on Github.  The master project repository is the organization called "espruino" that can be found here: `https://github.com/espruino`

The leader/founder/brains/owner behind the whole shooting match is Gordon Williams.
Within the espruino Github organization, we will find a Github repository called "Espruino".

`https://github.com/espruino/Espruino`

This is the master repository for the project.
Within this repository there is a branch called "ESP32".  It is against this branch we will be performing our work.
Note: The thinking is that we will perform all changes necessary within that branch so that there will be no breakage of the master work should something go wrong.  The ultimate goal is that at some date in the future, the majority of the work against an Espruino port to ESP32 will be considered "ready" and the work performed in the "ESP32" branch will be merged into the "master" branch and then on-going maintenance performed there.

Let us assume (as is actually the case at the time of writing) that the ESP32 branch does not yet exist.  The first step will be someone with suitable authorization create that branch.  Note that an Espruino on ESP32 developer will not be performing these steps.  They are recorded here for interest and for potential future projects.
The recipe to perform the branch creation task is as follows:

* Clone a local copy of Espruino
```
$ git clone https://github.com/espruino/Espruino.git
```

* Change into the directory for the clone
```
$ cd Espruino
```

* Create a new branch
```
$ git branch ESP32
```

* Checkout the branch
```
$ git checkout ESP32
```

* Push the changes back to master in Github
```
$ git push -u origin --all
```

And now we will have created a branch.

## Coding standards

<Do we have these already written?>
No tabs, indent by two spaces

## Build instructions
In order to build Espruino for an ESP32, I am assuming you have a Linux environment at your disposal.  While there is no technical reason you can't build Espruino on a different operating system, these notes assume Linux.  If you don't have a Linux platform, consider using a Raspberry Pi or VirtualBox.
We will need three major environmental components in order to build Espruino.  These are:

* An Xtensa ESP32 tool chain
* A copy of the Espressif IoT Development Framework (ESP-IDF)
* A copy of an ESP-IDF template application

### Let us now break these down.
The first is the Xtensa tool chain.  The processor on an ESP32 is made by Xtensa and has its own architecture and instruction set.  When we compile Espruino, we will be generating binary files that are the compiled C files.   These binary files contain the executable instructions and data that will eventually be loaded on the ESP32.  A compiler takes our source code and builds the resulting compiled code.  Our source is C language in text files and our result must be Xtensa binaries.  A tool chain is a suite of tools that take us from the source to the compiled binaries.  These include C compilers, linkers, archivers and a few others.  Espressif makes available a down loadable version of the Xtensa tool chain for Linux on x86 or x86-64.  If you have previously been working with the ESP8266 toolchains, make sure that you realize that the toolchain for ESP32 is distinct from that of the ESP8266.  The source code for the compilers is also available and can be downloaded and compiled yourself.  I had to do this to get the tool chain running on the ARM architecture for my Raspberry Pi.  I don't recommend building your own compiler unless you have a good reason.  Instead find a binary version and download and trust that it won't do mischief on your machine.
In my recipes I choose to install the tool chain in /opt/xtensa-esp32-elf and add /opt/xtensa-esp32-elf/bin to your environmental PATH.

Next we need the ESP-IDF.  This is the core of the framework that is the foundation upon which we build Espruino.  
This package provides the libraries we link against in order to leverage the services of the ESP32 environment.
The package contains modules for WiFi, TCP/IP sockets, GPIO and much more.  The package is distributed from Github and contains
pre-built libraries distributed in binary format as well as many modules that are distributed in source and need to be compiled.

Next we need an instance of an ESP-IDF template application.  This is another project type that is extractable from Github.
The true purpose of this project type is to be the basis for one of your own ESP32 applications.  The intent is that you download this
template application, rename it to reflect the name you want for your target app and then start editing the source for your own
purposes.  It also contains a Makefile which knows how to build a full binary ready for flashing onto the ESP32.
At this point you may be thinking to yourself that this seems a strange component to include
in our Espruino story … but here is the thinking.  The ESP-IDF is provided partially in binary format and partially
as source files that need compiled.  Because of this, we would need to compile the ESP-IDF to be able to have all 
the linkable libraries.  Unfortunately, as of this time, there doesn't appear to be a recipe to simply "build all the
ESP-IDF libraries".  Instead, it is the template application which builds them for us.  When we download a
ready-to-compile template and ask it to build, the result is not only a compiled application … but a side
effect is that all the ESP-IDF libraries we need are also compiled and placed in directories for us to link
against.  While it might be feasible to incorporate these rules to build all the ESP-IDF libraries that
Espruino requires into the Espruino Makefile, at this time we have decided that there is no immediate need to do that.
If we download the ESP-IDF and download the ESP-IDF template application and run a compile against that application,
the result (from Espruino build perspective) is that we have all the libraries we need to link against with minimal work
on our part.  Thankfully, the recipe to achieve these tasks is very easy and very repeatable.

In early version of Espruino port, SDK for esp32 did not support I2C and SPI.
Only option at that time was to use Arduino related driver.
This changed with new SDK which supports I2C, I2S, SPI and a lot of other driver.
Therefore we don't need Arduino driver anymore. If somebody still wants to use them, we will keep a description at the end of this document.

Here are the instructions that need only be followed once.
* In your home directory (or wherever you want the root of your build to be) create a directory called "esp32".
```
$ mkdir esp32
```

* Change into that directory.  This directory is what we will refer to as the ESP32 root directory in our story.
```
$ cd esp32
```

* Extract the ESP-IDF from Github
```
$ git clone --recursive https://github.com/espressif/esp-idf.git
```

* Change into the newly created esp-idf folder.
```
$ cd esp-idf
```

* Update the submodule.
```
$ git submodule update --init
```

* Change back up to the ESP32 root directory.
```
$ cd ..
```

* Extract the ESP-IDF template application to a folder called "template".
```
$ git clone https://github.com/espressif/esp-idf-template.git template
```

* Edit a file called setenv.sh and add the following lines:
```
#!/bin/bash
export ESP_IDF_PATH=$(pwd)/esp-idf
export IDF_PATH=$(pwd)/esp-idf
export ESP_APP_TEMPLATE_PATH=$(pwd)/template
export ESP32=1
[[ ":$PATH:" != *":/opt/xtensa-esp32-elf/bin:"* ]] && PATH="/opt/xtensa-esp32-elf/bin:${PATH}"
```

* Make the script executable:
```
$ chmod u+x setenv.sh
```

* Execute the environment script to set the variables.  You will need to re-run this each time you open a new shell for building Espruino.
```
$ . ./setenv.sh
```

* Change into the new template directory.
```
$ cd template
```

* If you need Arduino, you have to do it here with commands at the end of this document

* Run the `make menuconfig`.
```
$ make menuconfig
```

* Change some of the settings necessary for Espruino:
```
Component config -> LWIP -> Enable SO_REUSEADDR option [Enable]
Component config -> ESP32-specific config ->  Task watchdog [Disable]
            sooner or later we need to find a way to refresh Task watchdog
Component config -> FreeRTOS -> Halt when an SMP-untested function is called [Disable]
            some functions are marked as "SMP-untested" and a call results in a reset
            on our test they work fine, and for testing we need some of them
```

* Perform a build to create the libraries and the template application.
```
$ make
```

* Change back up to the ESP32 root directory.
```
$ cd ..
```

* Extract Espruino source from YOUR fork of Espruino:
```
$ git clone https://github.com/YOURGITHUBID/Espruino.git
```

* Change into the Espruino folder.
```
$ cd Espruino
```

$ Build Espruino:
```
$ make
```

If all has built correctly, you will now find that you have a flash-able binary representing Espruino for the ESP32.

In summary, here are the steps again without commentary:

```
$ mkdir esp32
$ cd esp32
$ git clone --recursive https://github.com/espressif/esp-idf.git
$ cd esp-idf
$ git submodule update --init
$ cd ..
$ git clone https://github.com/espressif/esp-idf-template.git template
$ nano setenv.sh
>>>
#!/bin/bash
export ESP_IDF_PATH=$(pwd)/esp-idf
export IDF_PATH=$(pwd)/esp-idf
export ESP_APP_TEMPLATE_PATH=$(pwd)/template
export ESP32=1
[[ ":$PATH:" != *":/opt/xtensa-esp32-elf/bin:"* ]] && PATH="/opt/xtensa-esp32-elf/bin:${PATH}"
<<<
$ chmod u+x setenv.sh
$ . ./setenv.sh
$ cd template
$ make menuconfig
$ make
$ cd ..
$ git clone https://github.com/YOURGITHUBID/Espruino.git
$ cd Espruino
$ make
```

### Include Arduino driver###
* Make the components folder.
```
$ mkdir components
```

* Change into the components folder.

```
$ cd components
```

* Retrieve the Arduino-esp32 project

```
$ git clone https://github.com/espressif/arduino-esp32.git
```

* Edit the `main.cpp` source file
```
$ nano arduino-esp32/cores/esp32/main.cpp
>>> Comment out app_main
/*
extern "C" void app_main()
{
    init();
    initVariant();
    initWiFi();
    xTaskCreatePinnedToCore(loopTask, "loopTask", 4096, NULL, 1, NULL, 1);
}
*/
<<<
```

* Change back up to the template folder
```
$ cd ..
```

