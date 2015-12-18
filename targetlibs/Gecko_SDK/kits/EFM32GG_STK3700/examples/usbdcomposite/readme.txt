USB Composite Device example, MSD + CDC + Vendor unique functions.

This example project use the EFM32 USB Device protocol stack
to implement a composite USB device with tree functions:

 - Mass storage class device (MSD) implementing a disk in internal flash memory.
 - Communication device class (CDC) implementing a USB<->UART bridge
 - Vendor Unique Device similar to the usbdvud example (control STK leds).

As the CDC function has two interface, the CDC function use an Interface
Association Descriptor (IAD). A composite device which use IAD's must be
implemented using bDeviceClass=0xEF, bDeviceSubClass=2 and bDeviceProtocol=1.


The vendor unique function (VUD).
=================================
Toggles user LED's 0 and 1 when receiving vendor unique class setup commands.
Intended to be used together with the "libusb" device driver, and host
application EFM32-LedApp.exe (a Windows application).
You will find libusb and EFM32-LedApp.exe in the "host" folder of the usbdvud
example. Check EFM32_Vendor_Unique_Device.inf to see how both the usbdvud
example VID_10C4&PID_0001 and this composite example VID_10C4&PID_0008&MI_00
are specified.

The CDC COM port function.
==========================
Implements an USB CDC based virtual COM port. USART1 on the DK is used as the
physical COM port. Any data sent to the virtual CDC COM port is transmitted on
USART1. Any data received on USART1 is transmitted to the virtual port.
USART1 is available on the STK EXT port. EXT pin 4 is Tx, pin 6 is Rx.
NOTE: This is a TTL level USART, DO NOT CONNECT DIRECTLY TO RS232 PORTS !

USB Mass Storage Device function.
=================================
Implements a Mass Storage Class device (MSD) with 4MByte in external PSRAM.


Windows driver installation.
============================
The first time the composite USB device is connected to the host, Windows must
install drivers for the VUD and CDC functions.

To control the VUD function with EFM32-LedApp.exe you need to install "libusb"
device driver. Manually direct Windows to look for this driver in the "host"
folder of the usbdvud example.
This can be done with the new device "Wizard" which might pop up after device
insertion, or you can open "Device Manager", left click on one of functions
of the new composite device (marked with yellow exclamation mark) and
select "Update Driver Software...".
After libusb is installed you can start EFM32-LedApp.exe to control leds on
the DK.

Similarly for the CDC function, direct Windows to look for a driver in the
folder where you have your copy of the "EFM32-Cdc.inf" file (same folder as
this readme file, unzip the Silabs-CDC_Install.zip).
Note how the .inf file specifies interface number 2, VID_10C4&PID_0008&MI_02.
When the serial port driver is succesfully installed, the device will be listed
as a "Ports" device in Device Manager, double-click it, select the
"Port Settings" tab and maybe the "Advanced..." button to set serial port
properties.


Some versions of Windows wont allow you to install unsigned drivers. If you
suspect this, reboot the PC and repeatedly push F8 during boot until the boot
menu appears. Select the "Disable Driver Signature Enforcement" option.
You should now be able to install unsigned drivers.


Board:  Silicon Labs EFM32GG-STK3700 Development Kit
Device: EFM32GG990F1024
