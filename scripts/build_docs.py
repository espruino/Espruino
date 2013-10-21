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
# Builds HTML documentation for functions from the JSON included in comments in
# jswrap_*.c files
# ----------------------------------------------------------------------------------------

import subprocess;
import re;
import json;
import sys;
import os;
sys.path.append(".");
import common

# Scans files for comments of the form /*JSON......*/ and then writes out an HTML file describing 
# all the functions

jsondatas = common.get_jsondata(True)

classes = []
for jsondata in jsondatas:
  if "class" in jsondata:
    if not jsondata["class"] in classes:
      classes.append(jsondata["class"])

htmlFile = open('functions.html', 'w')
def html(s): htmlFile.write(s+"\n");

def htmlify(d):
  d = re.sub(r'```([^`]+)```', r'<code>\1</code>', d) # code tags
  d = re.sub(r'(http://[^ ]+)', r'<a href="\1">\1</a>', d) # links tags
  return d

def html_description(ds,current):
  if not isinstance(ds, list): ds = [ ds ]
  for d in ds:
    for link in links:
      if link!=current:
        d = d.replace(" "+link+" ", " <a href=\"#"+links[link]+"\">"+link+"</a> ")
        d = d.replace(" "+link+".", " <a href=\"#"+links[link]+"\">"+link+"</a>.")
        d = d.replace(" "+link+"(", " <a href=\"#"+links[link]+"\">"+link+"</a>(")    
    html("   <p class=\"description\">"+htmlify(d)+"</p>")

def get_prefixed_name(jsondata):
  s=""
  if "class" in jsondata:
    s=s+jsondata["class"]+"."
  s=s+jsondata["name"]
  return s

def get_fullname(jsondata):
  if jsondata["type"]=="constructor":
    s = "constructor "
  else:
    s = "function "
    if "class" in jsondata:
      s=s+jsondata["class"]+"."
  s=s+jsondata["name"]
  return s

def get_surround(jsondata):
  if jsondata["type"]=="constructor":
    s = "constructor "
  else:
    s = "function "
    if "class" in jsondata:
      s=s+jsondata["class"]+"."
  s=s+jsondata["name"]
  if not common.is_property(jsondata):
    args = [];
    if "params" in jsondata:
      for param in jsondata["params"]:
        args.append(param[0]);
        if param[1]=="JsVarArray": args.append("...");
    s=s+"("+",".join(args)+")"
  return s

def get_link(jsondata):
  s="l_";
  if "class" in jsondata:
    s=s+jsondata["class"]+"_"
  else:
    s=s+"_global_"
  s=s+jsondata["name"]
  return s

html("<html>")
html(" <head>")
html("  <title>Espruino Reference</title>")
html("  <style>")
html("   body { font: 71%/1.5em  Verdana, 'Trebuchet MS', Arial, Sans-serif; color: #666666; }")
html("   h1, h2, h3, h4 { color: #000000; margin-left: 0px; }")
html("   h4 { padding-left: 20px; }")
html("   .class { page-break-before: always; width:95%; border-top: 1px solid black; border-bottom: 1px solid black; padding-top: 20px; padding-bottom: 20px; margin-top: 50px; }")
html("   .instance { font-weight: bold; }");
html("   .detail { width:90%; border-bottom: 1px solid black; margin-top: 50px; }")
html("   .top { float:right; }")
html("   .call { padding-left: 50px; }")
html("   .description { padding-left: 50px; }")
html("   .param { padding-left: 50px; }")
html("   .return { padding-left: 50px; }")
html("  </style>")
html(" </head>")
html(" <body>")
html("  <h1>Espruino Software Reference</h1>")
html("  <p style=\"text-align:right;\">Version "+common.get_version()+"</p>")
html("  <h2><a name=\"contents\">Contents</a></h2>")
html("  <h3><a href=\"#_global\">Global Functions</A></h3>")
html("  <ul>")
detail = []
links = {}
jsondatas = sorted(jsondatas, key=common.get_name_or_space)
for jsondata in jsondatas:
  if "name" in jsondata and not "class" in jsondata and not jsondata["type"]=="object":
    html("    <li><a href=\"#"+get_link(jsondata)+"\">"+get_surround(jsondata)+"</a></li>")
    if not "no_create_links" in jsondata:
      links[get_prefixed_name(jsondata)] = get_link(jsondata)
    detail.append(jsondata)
for className in sorted(classes):
  html("  </ul>")
  html("   <h3><a href=\"#"+className+"\">"+className+"</a></h3>")
  html("  <ul>")
  for jsondata in jsondatas:
    if "name" in jsondata and "class" in jsondata and jsondata["class"]==className:
      html("    <li><a href=\"#"+get_link(jsondata)+"\">"+get_surround(jsondata)+"</a></li>")
      if not "no_create_links" in jsondata:
        links[get_prefixed_name(jsondata)] = get_link(jsondata)
      detail.append(jsondata)
html("  </ul>")

html("  <h2>Detail</h2>")
lastClass = "XXX"
for jsondata in detail:
  className = ""
  niceName = ""
  linkName = ""
  if "class" in jsondata: 
    className=jsondata["class"]
    niceName=className+" Class"
    linkName=className
  else: 
    className=""                           
    niceName="Global Functions"
    linkName="_global"

  if className!=lastClass:
    lastClass=className
    html("<h2 class=\"class\"><a name=\""+linkName+"\">"+niceName+"</a></h2>")
    html("  <p class=\"top\"><a href=\"#top\">(top)</a></p>")
    for j in jsondatas:
      if (j["type"]=="class" or j["type"]=="library") and j["class"]==className:
        ds = html_description(j["description"], className)

    instances = []
    for j in jsondatas:
      if "instanceof" in j and j["instanceof"]==className:
        instances.append(j)
    if len(instances)>0:
      html("  <h4>Instances</h4>")
      html("  <ul>")
      for j in instances:
        html("    <li><p class=\"instance\">"+j["name"]+"</p>");
        html_description(j["description"], j["name"])
        html("    </li>")
      html("  </ul>")
    
    html("  <h4>Methods and Fields</h4>")
    html("  <ul>")
    for j in jsondatas:
      if ("name" in j) and (className!="" or not "instanceof" in j) and ((className=="" and not "class" in j) or ("class" in j and j["class"]==className)):
        html("    <li><a href=\"#"+get_link(j)+"\">"+get_surround(j)+"</a></li>")
    html("  </ul>")
  html("  <h3 class=\"detail\"><a name=\""+get_link(jsondata)+"\">"+get_fullname(jsondata)+"</a></h3>")
  html("  <p class=\"top\"><a href=\"#top\">(top)</a></p>")
  html("  <h4>Call type:</h4>")
  html("   <p class=\"call\">"+get_surround(jsondata)+"</p>")
  if "description" in jsondata:
    html("  <h4>Description</h4>")
    html_description(jsondata["description"], jsondata["name"])
  html("  <h4>Parameters</h4>")
  if "params" in jsondata:
    for param in jsondata["params"]:
      desc = ""
      if len(param)>2: desc=param[2]
      if isinstance(desc, list): desc = '<br/>'.join(desc)
      extra = ""
      if  param[1]=="JsVarArray": extra = ", ...";
      html("   <p class=\"param\"><b> "+param[0]+extra+"</b> "+htmlify(desc)+"</p>")
  else:
    html("   <p class=\"param\">No parameters</p>")
  html("  <h4>Returns</h4>")
  if "return" in jsondata:
    desc = ""
    if len(jsondata["return"])>1: desc=jsondata["return"][1]
    if desc=="": desc="See description above"
    html("   <p class=\"return\">"+htmlify(desc)+"</p>")
  else:
    html("   <p class=\"return\">No return value (undefined)</p>")

html(" </body>")
html("</html>")



# --------------------------------------------------------------------------
#                                                     Write/create keywords
# --------------------------------------------------------------------------
keywords = {}
for j in jsondatas:
  if ("name" in j):
    item = { "title" : get_surround(j), "path": "/Reference#"+get_link(j) };
    jkeywords = [ j["name"] ]
    if get_prefixed_name(j)!=j["name"]: jkeywords.append(get_prefixed_name(j))
    if "class" in j: jkeywords.append(j["class"])
    
    for k in jkeywords:
      k = k.lower()
      if not k in keywords: 
        keywords[k] = [ item ]
      else:
        keywords[k].append(item)

#print(json.dumps(keywords, sort_keys=True, indent=2)) 
keywordFile = open('function_keywords.js', 'w')
keywordFile.write(json.dumps(keywords, sort_keys=True, indent=2));


