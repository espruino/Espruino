USB Vendor Unique Device example.

This example project use the EFM32 USB Device protocol stack
and implements an USB Vendor Unique Device.

The example will toggle LED's 0 and 1 when receiving vendor unique class
setup commands.

This example is intended to be used together with the "libusb" device driver,
and host application EFM32-LedApp.exe (a Windows application).

The first time the USB device is connected to the host, you must tell the host
OS to install the "libusb" device driver.

On Windows, you can follow this procedure:
Insert the device, and manually direct Windows to look for drivers in the
directory where you have your copy of the "EFM32_Vendor_Unique_Device.inf" file.
This can be done with the new device "Wizard" which might pop up after device
insertion, or you can open "Device Manager", left click on the new device and
select "Update Driver Software...".

Some versions of Windows wont allow you to install unsigned drivers. If you
suspect this, reboot the PC and repeatedly push F8 during boot until the boot
menu appears. Select the "Disable Driver Signature Enforcement" option.
You should now be able to install an unsigned driver.

After libusb is installed, start EFM32-LedApp.exe.

Board:  Silicon Labs EFM32GG-STK3700 Development Kit
Device: EFM32GG990F1024
