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
# Builds HTML documentation from the files in the boards directory
# ----------------------------------------------------------------------------------------

import subprocess;
import re;
import json;
import sys;
import os;
import importlib;

scriptdir = os.path.dirname(os.path.realpath(__file__))
basedir = scriptdir+"/../"
sys.path.append(basedir+"scripts");
sys.path.append(basedir+"boards");

import pinutils;

# -----------------------------------------------------------------------------------------

# Now scan AF file
print "Script location "+scriptdir
#if len(sys.argv)!=3:
#  print "ERROR, USAGE: build_board_docs.py BOARD_NAME HTML_FILENAME"
#  exit(1)
if len(sys.argv)!=2:
  print "ERROR, USAGE: build_board_docs.py BOARD_NAME"
  exit(1)
boardname = sys.argv[1]
#htmlFilename = sys.argv[2]
htmlFilename = "boards/"+boardname+".html"
print "HTML_FILENAME "+htmlFilename
print "BOARD "+boardname
# import the board def
board = importlib.import_module(boardname)
# Call the included board_specific file - it sets up 'pins' and 'fill_gaps'
pins = board.get_pins()
pins = pinutils.append_devices_to_pin_list(pins, board)

# -----------------------------------------------------------------------------------------
for pin in pins:
  if pin["name"][0] == 'P':
    pin["name"] = pin["name"][1:];

pinmap = {};
if '_pinmap' in board.board:
  pinmap = board.board['_pinmap'];

# -----------------------------------------------------------------------------------------
htmlFile = open(htmlFilename, 'w')
def writeHTML(s): htmlFile.write(s+"\n");


def dump_pin(pin, pinstrip):

      if pin in pinmap:
        pin = pinmap[pin];      
      pininfo = pinutils.findpin(pins, pin, False)


      not_five_volt = False
#      print(json.dumps(pininfo))
      if ("csv" in pininfo) and ("IO" in pininfo["csv"]) and  ("Type" in pininfo["csv"]) and (pininfo["csv"]["Type"]=="I/O") and (pininfo["csv"]["IO"]!="FT") : 
         not_five_volt = True

      writeHTML('    <DIV class="'+pinstrip+'pin pin">');
      pinHTML = '     <SPAN class="pinname">'+pin+"</SPAN>";
      pinHTML2 = '';

      if not_five_volt:
        pinHTML2 += '<SPAN class="pinfunction NOT_5V" title="Not 5v Tolerant">3.3v</SPAN>';

      reverse = pinstrip=="left" or pinstrip=="right2";
      if not reverse: writeHTML(pinHTML+"\n"+pinHTML2)

      pinfuncs = {}

      for func in sorted(pininfo["functions"]):
#       writeHTML('     '+func)    
        if func in pinutils.CLASSES:       
          funcdata = str(pininfo["functions"][func])
          cls = pinutils.CLASSES[func]
          name = cls
          title = func
          if cls=="I2C" or cls=="SPI" or cls=="USART": name=func.replace("_"," ")
          
          if cls=="DEVICE" and funcdata[:4]=="pin_":
            title = title + " ("+funcdata[4:]+")";
#            print title
          if func in pinutils.NAMES: name = pinutils.NAMES[func]
          writeHTML('<!-- '+func+' -->')
          if name in pinfuncs:
            pinfuncs[name]["title"] = pinfuncs[name]["title"] + " " + title
          else:
            pinfuncs[name] = { 'cls': cls, 'title': "["+pin+"] "+title, 'name': name, 'id': pin+"_"+func };

      for func in sorted(pinfuncs.items(),key=lambda x: x[1]['cls']):
        pf = func[1]
        writeHTML('     <SPAN class="pinfunction '+pf["cls"]+'" title="'+pf["title"]+'" onMouseOver="showTT(\''+pf["id"]+'\')" onMouseOut="hideTT(\''+pf["id"]+'\')">'+pf["name"]+'</SPAN>')
        writeHTML('     <SPAN class="pintooltip" id="'+pf["id"]+'" style="display:none;">'+pf["title"]+'</SPAN>')        
          
      if reverse: writeHTML(pinHTML2+"\n"+pinHTML)
      writeHTML('    </DIV>')    


writeHTML("""
<HTML>
 <HEAD>
  <STYLE>
   #boardcontainer { position: relative; }
   #board { 
     position: absolute; 
     background-size: 100% auto; # width and height, can be %, px or whatever.
   }
   .pin { padding: 1px; height: 20px;  }
   .pinname { 
      background-color: #FFF;
      border:1px solid black; 
      padding-left: 2px;
      padding-right: 2px; 
      font-weight: bold;      
    }
   .pinfunction { 
     border:1px solid black; 
     border-radius:3px; 
     padding-left: 2px;
     padding-right: 2px; 
   }
   .pintooltip {
      background-color: #FFD;
      border:1px solid black; 
      padding-left: 2px;
      padding-right: 2px; 
      font-weight: bold;  
      position: absolute;
      z-index: 100;
   }
   .SPI { background-color: #8F8; }
   .ADC { background-color: #88F; }
   .DAC { background-color: #0CC; }
   .PWM { background-color: #8FF; }
   .USART { background-color: #FF8; }
   .CAN { background-color: #8CC; }
   .I2C { background-color: #F88; }
   .DEVICE { background-color: #F8F; }
   .NOT_5V { background-color: #FDD; }

#top { white-space: nowrap; }
#top2 { white-space: nowrap; }
#bottom { white-space: nowrap; }
#bottom2 { white-space: nowrap; }
#left { text-align:right; }
#right2 { text-align:right; }
.toppin {
  -webkit-transform: rotate(-90deg);
  -moz-transform: rotate(-90deg);
  -ms-transform: rotate(-90deg);
  -o-transform: rotate(-90deg);
  transform: rotate(-90deg);
  display: inline-block;
  width: 20px;
}
.top2pin {
  -webkit-transform: rotate(90deg);
  -moz-transform: rotate(90deg);
  -ms-transform: rotate(90deg);
  -o-transform: rotate(90deg);
  transform: rotate(90deg);
  display: inline-block;
  width: 20px;
}
.bottompin {
  -webkit-transform: rotate(90deg);
  -moz-transform: rotate(90deg);
  -ms-transform: rotate(90deg);
  -o-transform: rotate(90deg);
  transform: rotate(90deg);
  display: inline-block;
  width: 20px;
}
.bottom2pin {
  -webkit-transform: rotate(-90deg);
  -moz-transform: rotate(-90deg);
  -ms-transform: rotate(-90deg);
  -o-transform: rotate(-90deg);
  transform: rotate(-90deg);
  display: inline-block;
  width: 20px;
}


""");
for pinstrip in board.board:
  writeHTML("   #"+pinstrip+" { position: absolute; }")
  writeHTML("   ."+pinstrip+"pin { white-space: nowrap; }")
writeHTML(board.board_css)
writeHTML("  </STYLE>")
writeHTML(""""
  <SCRIPT type="text/javascript"> 
    function showTT(ttid) { 
      var e = document.getElementById(ttid);
      e.style.display = "block";
    }
    function hideTT(ttid) { 
      var e = document.getElementById(ttid);
      e.style.display = "none";
    }
  </SCRIPT>
""")
writeHTML(" </HEAD>")
writeHTML(" <BODY>")
writeHTML('  <H1>'+board.info["name"]+'</H1>')
writeHTML('  <!-- '+boardname+' -->')
if "link" in board.info:
  for link in board.info["link"]:
    writeHTML('  <P><a href=\"'+link+'\"" target="_blank">'+link+'</a></P>')    
writeHTML('  <H2>Specifications</H2>')
writeHTML('  <TABLE style="margin-left:100px;">')
writeHTML('   <TR><TH width="256">Chip</TH><TD>'+board.chip['part']+'</TD></TR>')
writeHTML('   <TR><TH>Package</TH><TD>'+board.chip['package']+'</TD></TR>')
writeHTML('   <TR><TH>RAM</TH><TD>'+str(board.chip['ram'])+' kBytes</TD></TR>')
writeHTML('   <TR><TH>Flash</TH><TD>'+str(board.chip['flash'])+' kBytes</TD></TR>')
writeHTML('   <TR><TH>Speed</TH><TD>'+str(board.chip['speed'])+' Mhz</TD></TR>')
writeHTML('   <TR><TH>USARTs</TH><TD>'+str(board.chip['usart'])+'</TD></TR>')
writeHTML('   <TR><TH>SPIs</TH><TD>'+str(board.chip['spi'])+'</TD></TR>')
writeHTML('   <TR><TH>I2Cs</TH><TD>'+str(board.chip['i2c'])+'</TD></TR>')
writeHTML('   <TR><TH>USB</TH><TD>'+("Yes" if "USB" in board.devices else "No")+'</TD></TR>')
writeHTML('   <TR><TH>DACs</TH><TD>'+(str(board.chip['dac']) if board.chip['dac']>0 else "No")+'</TD></TR>')
writeHTML('   <TR><TH>SD Card</TH><TD>'+("Yes" if "SD" in board.devices else "No")+'</TD></TR>')
writeHTML('  </TABLE>')
writeHTML('  <P>Like this? Please tell your friends, blog, or <a href="http://www.espruino.com/kick">support us on KickStarter</a>!</P>')
writeHTML('  <H2>Pinout</H2>')
writeHTML('  <P>Hover the mouse over a pin function for more information</P>')
writeHTML('  <DIV id="boardcontainer">')
writeHTML('  <DIV id="board">')
usedpins = []
for pinstrip in board.board:
  if pinstrip[0]!='_':
    writeHTML('   <DIV id="'+pinstrip+'">')
    for pin in board.board[pinstrip]:  
      usedpins.append(pin)
      dump_pin(pin, pinstrip)
    writeHTML('   </DIV>')    
writeHTML('  </DIV>')
writeHTML('  </DIV>')

otherpins=0
for pinstruct in pins:
  pin = pinstruct["name"]
  if not pin in usedpins: 
    otherpins = otherpins + 1

if otherpins>0:
  writeHTML('  <DIV id="otherpins">')
  writeHTML('   <H2>Pins not on connectors</H2>')
  for pinstruct in pins:
    pin = pinstruct["name"]
    if not pin in usedpins:    
      dump_pin(pin, "otherpins")
  writeHTML('  </DIV>')
writeHTML('  <P></P>')
writeHTML(" </BODY>")
writeHTML("</HTML>")

