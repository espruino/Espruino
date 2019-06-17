================ Silicon Labs USB STACK =====================================

This directory, "usb", contains the Silicon Labs USB stack for the EFM32
series of microcontrollers.

Some design guidelines for this library is:

* Follow the guidelines established by ARM's and Silicon Labs's adaptation
  of the CMSIS (see below) standard

* Be usable as a starting point for developing richer, more target specific
  functionality (i.e. copy and modify further)

* Ability to be used as a standalone software component, used by other drivers
  that should cover "the most common cases"

* Readability of the code and usability preferred before optimization for speed
  and size or covering a particular "narrow" purpose

* As little "cross-dependency" between modules as possible, to enable users to
  pick and choose what they want

================ About CMSIS ================================================

The USB APIs are built on top of EFM32 CMSIS headers and em_lib API'S, which
again are based on the Cortex Microcontroller Software Interface Standard.
These files are supplied together with this archive in separate folders.

The library requires basic C99-support. You might have to enable C99 support
in your compiler. Comments are in doxygen compatible format, and can be viewed
online from Silicon Labs's web site.

To download updates of this EFM32 CMSIS release, go to
    http://www.silabs.com/

For more information about CMSIS see
    http://www.onarm.com

================ File structure ==============================================

inc/ - header files
src/ - source files

================ License =====================================================

See the top of each file for SW license. Basically you are free to use the
Silicon Labs code for any project using Silicon Labs devices. Parts of the
CMSIS library is copyrighted by ARM Inc. See "License.doc" for ARM's CMSIS
license.

================ Software updates ============================================

Silicon Labs continually works to provide updated and improved example code,
header files and other software of use for our customers. Please check

http://www.silabs.com/support/pages/document-library.aspx?p=MCUs--32-bit

for the latest releases. If you download and install the
Simplicity Studio application, you will be notified about updates when
available.

(C) Copyright Silicon Labs, 2015
