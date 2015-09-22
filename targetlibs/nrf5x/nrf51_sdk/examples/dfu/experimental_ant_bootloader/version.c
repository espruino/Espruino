 /*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright (c) 2013 Dynastream Innovations Inc.
 * All rights reserved. This software may not be reproduced by
 * any means without express written approval of Dynastream
 * Innovations Inc.
 */

/*
 * NOTES:
 *
 * version "AAA#.##B##"
 *
 * SW_VER_MAJOR   - Increases on any released applicaion major feature update/changes or new features
 * SW_VER_MINOR   - Increases on any release application minor feature update i.e. Bug fixing and minor features.
 * SW_VER_PREFIX  - Is fixed on this firmware.
 * SW_VER_POSTFIX - Increases on any internal development builds. OR might be used for tagging special builds. OR might be used on branch build
  */


   #define     SW_VER_MAJOR      "1."
   #define     SW_VER_MINOR      "00"  // last change was merging in antfs updates

   #define     SW_VER_PREFIX     "BFD" // N548 Reference Design Bootloader
   #define     SW_VER_POSTFIX    "B00"


/***************************************************************************
*/
const char ac_bootloader_version[] = SW_VER_PREFIX SW_VER_MAJOR SW_VER_MINOR SW_VER_POSTFIX;  // Max 11 characters including null
