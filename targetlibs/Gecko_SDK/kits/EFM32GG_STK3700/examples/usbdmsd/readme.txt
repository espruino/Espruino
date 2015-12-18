USB Mass Storage Device example.

This example project use the EFM32 USB Device protocol stack
to implement a device Mass Storage Class device (MSD).

The example provides two different MSD types.

 - A 96KByte "disk" in internal SRAM (on devices with 128 kByte or larger SRAM)
 - An internal FLASH "disk" (disksize is internal FLASH size minus 64K)

Select mediatype in msddmedia.h (#define MSD_MEDIA)

Board:  Silicon Labs EFM32GG_STK3700 Development Kit
Device: EFM32GG990F1024
