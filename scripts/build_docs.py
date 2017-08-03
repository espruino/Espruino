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

# Needs:
#    pip install markdown
#    pip install markdown-urlize

import subprocess;
import re;
import json;
import sys;
import os;
sys.path.append(".");
import common
import urllib2
import markdown
import htmlentitydefs

# Scans files for comments of the form /*JSON......*/ and then writes out an HTML file describing
# all the functions

# htmldev default is False
# if htmldev is set to True indicating 'html development mode', script:
# - creates standalone html version for temporary html generation development by:
# --- inserting a searchbox text input to test top link function
# --- skipping MDN links dump and validation to complete quicker

htmldev = False

jsondatas = common.get_jsondata(True)

classes = []
libraries = []
for jsondata in jsondatas:
  if "class" in jsondata:
    if not jsondata["class"] in classes:
      classes.append(jsondata["class"])
  if jsondata["type"]=="library":
    if not jsondata["class"] in libraries:
      libraries.append(jsondata["class"])

# Load list of 'uses' in EspruinoDocs
code_uses = []
referenceFile = "../EspruinoDocs/references.json"
if os.path.isfile(referenceFile):
  print("Found references.json - using this to link to examples")
  code_uses = json.loads(open(referenceFile, "r").read())

# Load list of MDN URLs (to speed up processing)
valid_mdn_urls = { 'valid' : [], 'invalid' : [] };
mdnURLFile = "MDN_URLS.txt"
if os.path.isfile(mdnURLFile):
  valid_mdn_urls = json.loads(open(mdnURLFile, "r").read())

# start writing
htmlFile = open('functions.html', 'w')
def html(s): htmlFile.write(s+"\n");

def htmlify(d):
  d = markdown.markdown(d, extensions=['urlize'])
  # replace <code> with newlines with pre
  idx = d.find("<code>")
  end = d.find("</code>", idx)
  while idx>=0 and end>idx:
    codeBlock = d[idx:end+7]
    if codeBlock.find("\n")>=0:
      d = d[0:idx]+"<pre>"+codeBlock+"</pre>"+d[end+7:];
    idx = d.find("<code>", end+7+5+6)
    end = d.find("</code>", idx)
  return d;

def html_description(d,current):
  if isinstance(d, list): d = "\n".join(d)
  d = htmlify(d)
  for link in links:
    if link!=current:
      d = d.replace(" "+link+" ", " <a href=\"#"+links[link]+"\">"+link+"</a> ")
      d = d.replace(" "+link+".", " <a href=\"#"+links[link]+"\">"+link+"</a>.")
      d = d.replace(" "+link+"(", " <a href=\"#"+links[link]+"\">"+link+"</a>(")
  html("<div class=\"description\">\n" + d + "\n</div>\n")

def get_prefixed_name(jsondata):
  s=""
  if "class" in jsondata:
    s=s+jsondata["class"]+"."
  s=s+jsondata["name"]
  return s

def get_fullname(jsondata):
  s = common.get_prefix_name(jsondata)
  if s!="": s = s + " "
  if jsondata["type"]!="constructor":
    if "class" in jsondata:
      s=s+jsondata["class"]+"."
  s=s+jsondata["name"]
  return s

def get_arguments(jsondata):
  if common.is_property(jsondata):
    return ""
  args = [];
  if "params" in jsondata:
    for param in jsondata["params"]:
      args.append(param[0]);
      if param[1]=="JsVarArray": args.append("...");
  return "("+", ".join(args)+")"

def get_surround(jsondata):
  s = common.get_prefix_name(jsondata)
  if s!="": s = s + " "
  if jsondata["type"]!="constructor":
    if "class" in jsondata: s=s+jsondata["class"]+"."
  s=s+jsondata["name"]
  if jsondata["type"]!="object":
    s=s+get_arguments(jsondata)
  return s

def get_code(jsondata):
  if jsondata["type"]=="event":
    return jsondata["class"]+".on('"+jsondata["name"]+"', function"+get_arguments(jsondata)+" { ... });";
  if jsondata["type"]=="constructor":
    return "new "+jsondata["name"]+get_arguments(jsondata);
  return get_surround(jsondata)

def get_link(jsondata):
  s="l_";
  if "class" in jsondata:
    s=s+jsondata["class"]+"_"
  else:
    s=s+"_global_"
  s=s+jsondata["name"]
  return s

def html_escape(text):
  escaped_chars = ""
  for c in text:
    if (ord(c) < 32) or (ord(c) > 126):
      c = '&{};'.format(htmlentitydefs.codepoint2name[ord(c)])
    escaped_chars = escaped_chars + c
  return escaped_chars

# If MDN doesn't 404 then include a link to it
def insert_mdn_link(jsondata):
  if "class" in jsondata and "name" in jsondata:
    url = "https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/"+jsondata["class"]
    if jsondata["type"]!="constructor": url = url +"/"+jsondata["name"]
    if url in valid_mdn_urls['valid']:
      code = 200
    elif url in valid_mdn_urls['invalid']:
      code = 404
    else:
      print("Checking URL "+url)
      try:
        connection = urllib2.urlopen(url)
        code = connection.getcode()
        connection.close()
      except urllib2.HTTPError, e:
        code = e.getcode()
      if code==200: valid_mdn_urls['valid'].append(url)
      else: valid_mdn_urls['invalid'].append(url)
    if code==200:
      html("<p><a href=\""+url+"\">View MDN documentation</a></p>")

html("<html>")
html(" <head>")
html("  <title>Espruino Reference</title>")
html("  <style>")
html("   body { font: 71%/1.5em  Verdana, 'Trebuchet MS', Arial, Sans-serif; color: #666666; }")
html("   h1, h2, h3, h4 { color: #000000; margin-left: 0px; }")
html("   h4 { padding-left: 20px; }")
html("   ul { list-style-position: inside; }")
html("   .class { page-break-before: always; width:95%; border-top: 1px solid black; border-bottom: 1px solid black; padding-top: 20px; padding-bottom: 20px; margin-top: 50px; }")
html("   .instance { font-weight: bold; }");
html("   .detail { width:90%; border-bottom: 1px solid black; margin-top: 50px; }")
html("   .githublink { text-decoration:none; color:#CCC; }")
html("   .top { float:right; }")
html("   .call { padding-left: 50px; }")
html("   .description { padding-left: 50px; }")
html("   .param { padding-left: 50px; }")
html("   .return { padding-left: 50px; }")
html("   .examples { padding-left: 50px; }")
html("   .blush {")
html("     -webkit-transition: background 0.75s ease-out;")
html("     -moz-transition: background 0.75s ease-out;")
html("     -ms-transition: background 0.75s ease-out;")
html("     -o-transition: background 0.75s ease-out;")
html("     transition: background 0.75s ease-out;")
html("   }")
html("  .fwrdblush { background-color: gold; }")
html("  .bwrdblush { background-color: lightblue; }")
html("  </style>")
html("  <script>function vpos(nme,f) {")
html("  }")
html("  function blush(node){")
html("    var clazz = (node.name && node.name.substr(0,2) === \"t_\") ? \"bwrdblush\" : \"fwrdblush\";")
html("    node.className = node.className + \" \" + clazz;")
html("    setTimeout(function(){ node.className = node.className.replace(new RegExp(\"\\s*\" + clazz + \"\\s*\",\"g\"),\"\"); },750);")
html("  }")
html("  function place(nme){")
html("    var ns = document.getElementsByName(nme);")
html("    if (ns.length > 0) {")
html("      var n = ns[0], t = 0 - Math.floor(window.innerHeight * 0.2);")
html("      if (n.offsetParent) { do { t += n.offsetTop; } while ( n = n.offsetParent) }")
html("      blush(ns[0]);")
html("      setTimeout(function(){ window.scroll(0,(t < 0) ? 0 : t); },10);")
html("    }")
html("    return true;")
html("  }")
html("  function toppos(){")
html("    document.location=\"#top\"")
html("    var ns = document.getElementsByName(\"searchbox\"), n;")
html("    if ((ns.length > 0) && (typeof (n = ns[0]).value !== \"undefined\")) {")
html("      n.focus(); n.select();")
html("    }")
html("  }</script>")
html(" </head>")
html(" <body>")
html("  <h1>Espruino Software Reference</h1>")
html("  <p style=\"text-align:right;\">Version "+common.get_version()+"</p>")

if htmldev == True:
  html("  <div>")
  html("   <input class=\"searchbox\" name=\"searchbox\" size=\"60\" type=\"text\" autocomplete=\"off\">");
  html("  </div>")

detail = []
links = {}
jsondatas = sorted(jsondatas, key=lambda s: common.get_name_or_space(s).lower())

html('  <div id="contents">')
html("  <h2><a name=\"contents\"></a>Contents</h2>")
html("  <ul>")
html("  <li><a class=\"blush\" name=\"t__global\" href=\"#_global\" onclick=\"place('_global');\">Globals</A></li>")
for jsondata in jsondatas:
  if "name" in jsondata and not "class" in jsondata:
    link = get_link(jsondata)
    if not "no_create_links" in jsondata:
      links[get_prefixed_name(jsondata)] = link
    detail.append(jsondata)
for className in sorted(classes, key=lambda s: s.lower()):
  html("  <li><a class=\"blush\" name=\"t_"+className+"\" href=\"#"+className+"\" onclick=\"place('"+className+"');\">"+className+"</a></li>")
  for jsondata in jsondatas:
    if "name" in jsondata and "class" in jsondata and jsondata["class"]==className:
      link = get_link(jsondata)
      if not "no_create_links" in jsondata:
        links[get_prefixed_name(jsondata)] = link
      detail.append(jsondata)
html("  </ul>")
html('  </div><!-- Contents -->')

html("  <a class=\"blush\" name=\"top\"\>");
#html("  <h2>Detail</h2>")
lastClass = "XXX"
for jsondata in detail:
  className = ""
  niceName = ""
  linkName = ""
  if "class" in jsondata:
    className=jsondata["class"]
    if className in libraries:
      niceName=className+" Library"
    else:
      niceName=className+" Class"
    linkName=className
  else:
    className=""
    niceName="Globals"
    linkName="_global"

  if className!=lastClass:
    lastClass=className
    html("<h2 class=\"class\"><a class=\"blush\" name=\""+linkName+"\" href=\"#t_"+linkName+"\" onclick=\"place('t_"+linkName+"');\">"+niceName+"</a></h2>")
    html("  <p class=\"top\"><a href=\"javascript:toppos();\">(top)</a></p>")
    for j in jsondatas:
      if (j["type"]=="class" or j["type"]=="library") and j["class"]==className and "description" in j:
        html_description(j["description"], className)

    instances = []
    for j in jsondatas:
      if "instanceof" in j and j["instanceof"]==className:
        instances.append(j)
    if len(instances)>0:
      html("  <h4>Instances</h4>")
      text = ""
      for j in instances:
        text = text + " * [`"+j["name"]+"`](#l__global_"+j["name"]+")";
        if "description" in j: text = text + " " + j["description"]
        text = text + "\n"
      html_description(text, "")

    html("  <h4>Methods and Fields</h4>")
    html("  <ul>")
    for j in jsondatas:
      if ("name" in j) and (className!="" or not "instanceof" in j) and ((className=="" and not "class" in j) or ("class" in j and j["class"]==className)):
        link = get_link(j)
        html("    <li><a name=\"t_"+link+"\" href=\"#"+link+"\">"+get_surround(j)+"</a></li>")
    html("  </ul>")
  link = get_link(jsondata)
  html("  <h3 class=\"detail\"><a class=\"blush\" name=\""+link+"\" href=\"#t_"+link+"\" onclick=\"place('t_"+link+"','"+linkName+"');\">"+get_fullname(jsondata)+"</a>")
  html("<!-- "+json.dumps(jsondata, sort_keys=True, indent=2)+"-->");
  if "githublink" in jsondata:
    html('<a class="githublink" title="Link to source code on GitHub" href="'+jsondata["githublink"]+'">&rArr;</a>');
  html("</h3>")
  insert_mdn_link(jsondata);
  html("  <p class=\"top\"><a href=\"javascript:toppos();\">(top)</a></p>")
  if jsondata["type"]!="object":
    html("  <h4>Call type:</h4>")
    html("   <div class=\"call\"><code>"+get_code(jsondata)+"</code></div>")
  elif "instanceof" in jsondata:
    html("   <h4>Instance of <a href=\"#"+jsondata["instanceof"]+"\"><code>"+jsondata["instanceof"]+"</code></a>")
  if "description" in jsondata:
    html("  <h4>Description</h4>")
    desc = jsondata["description"]
    if not isinstance(desc, list): desc = [ desc ]
    if ("ifdef" in jsondata) or ("ifndef" in jsondata):
      if "ifdef" in jsondata: 
        desc.append("\n\n**Note:** This is only available in "+common.get_ifdef_description(jsondata["ifdef"]));
      if "ifndef" in jsondata:
        desc.append("\n\n**Note:** This is not available in "+common.get_ifdef_description(jsondata["ifndef"]));      
    html_description(desc, jsondata["name"])
  if "params" in jsondata:
    html("  <h4>Parameters</h4>")
    for param in jsondata["params"]:
      desc = ""
      if len(param)>2: desc=param[2]
      if isinstance(desc, list): desc = '<br/>'.join(desc)
      extra = ""
      if  param[1]=="JsVarArray": extra = ", ...";
      html("   <div class=\"param\">"+htmlify("`"+param[0]+extra+"` - "+desc)+"</div>")
  if "return" in jsondata:
    html("  <h4>Returns</h4>")
    desc = ""
    if len(jsondata["return"])>1: desc=jsondata["return"][1]
    if desc=="": desc="See description above"
    html("   <div class=\"return\">"+htmlify(desc)+"</div>")

  url = "http://www.espruino.com/Reference#"+get_link(jsondata)
  if url in code_uses:
    uses = code_uses[url]
    html("  <h4>Examples</h4>")
    html("  <p class=\"examples\">This function is used in the following places in Espruino's documentation</p>")
    html("  <ul class=\"examples\">")
    for link in uses:
      html('    <li><a href="'+link["url"]+'">'+html_escape(link["title"])+'</a></li>')
    html("  </ul>")

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


# ---------------------------- Just random helper stuff
builtins = []
for jsondata in jsondatas:
  if jsondata["type"]=="staticmethod" or jsondata["type"]=="staticproperty" or jsondata["type"]=="class":
    if not jsondata["class"] in builtins:
      builtins.append(jsondata["class"])
  elif jsondata["type"]=="function" or jsondata["type"]=="variable" or jsondata["type"]=="class":
      if not jsondata["name"] in builtins:
        builtins.append(jsondata["name"])
print("------------------------------------------------------")
print('Global classes and functions: '+' '.join(builtins));
print("------------------------------------------------------")

# Writing MDN URL file
if htmldev == False:
  open(mdnURLFile, "w").write(json.dumps(valid_mdn_urls, sort_keys=True, indent=2))
