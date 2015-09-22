/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/



#ifndef N5SK_BUTTON_H_
#define N5SK_BUTTON_H_

#include "stdint.h"


////////////////////////////////////////////////////////////////////////////////
// Module Description
////////////////////////////////////////////////////////////////////////////////

/*
 * This module is responsible for initializing the N5 Starter Kit IO board
 * buttons and Battery Board switches as inputs
 */

////////////////////////////////////////////////////////////////////////////////
// Public Function Prototypes
////////////////////////////////////////////////////////////////////////////////
void
n5_io_button_init(void);

void
n5_io_switch_init(void);

#endif /* N5SK_BUTTON_H_ */
