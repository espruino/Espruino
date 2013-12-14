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

import subprocess;
import re;
import json;
import sys;
import os;
import common;

def get_struct_from_jsondata():
  schemas = common.get_jsondata(True)
  #print(json.dumps(schemas, sort_keys=True, indent=2))

  context = dict()

  def checkClass(schema):
    cl = schema["class"]
    if not cl in context:
      context[cl] = {"methods": {}, "props": {}, "staticprops": {}, "staticmethods": {}, "desc": schema["description"]}
    return cl

  def addMethod(cl, details, type = ""):
    context[cl][type + "methods"][details["name"]] = {"params": details.get("params", []), "return": details.get("return", [])}

  def addProp(cl, details, type = ""):
    context[cl][type + "props"][details["name"]] = {"return": details.get("return", [])}

  for schema in schemas:
    type = schema["type"]
    if type=="class":
      checkClass(schema)
    elif type=="method":
      addMethod(checkClass(schema), schema)
    elif type=="property":
      addProp(checkClass(schema), schema)
    elif type=="staticmethod":
      addMethod(checkClass(schema), schema, "static")
    elif type=="staticproperty":
      addProp(checkClass(schema), schema, "static")

  return context

print(json.dumps(get_struct_from_jsondata(), sort_keys=True, indent=2))

