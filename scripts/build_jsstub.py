#!/usr/bin/python

# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ----------------------------------------------------------------------------------------
# Reads JSON schemas from jswrap files and uses it to generate a JS stub
# which is useful for autocomplete
# ----------------------------------------------------------------------------------------

import json;
import common;

schemas = common.get_jsondata(True)
context = common.get_struct_from_jsondata(schemas)
print(json.dumps(context, sort_keys=True, indent=2))

