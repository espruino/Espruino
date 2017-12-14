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
print("Script location "+scriptdir)
embeddable = False
boardname = ""
if len(sys.argv)==3 and sys.argv[2]=="pinout":
  embeddable = True 
  boardname = sys.argv[1]
if len(sys.argv)==2:
  boardname = sys.argv[1]
if boardname=="":
  print("ERROR...")
  print("USAGE: build_board_docs.py BOARD_NAME [pinout]")
  print("          'pinout' will output embeddable HTML of just the pinout")
  exit(1)

print("BOARD "+boardname)

#htmlFilename = sys.argv[2]
htmlFilename = "boards/"+boardname+".html"
print("HTML_FILENAME "+htmlFilename)
htmlFile = open(htmlFilename, 'w')
def writeHTML(s): htmlFile.write(s+"\n");

# import the board def
board = importlib.import_module(boardname)
# Call the included board_specific file - it sets up 'pins' and 'fill_gaps'
pins = board.get_pins()
pins = pinutils.append_devices_to_pin_list(pins, board)
pins = pinutils.remove_used_pinfunctions(pins, board)

#if not embeddable and  "link" in board.info and board.info["link"][0].startswith("http://www.espruino.com"):
#  writeHTML('<html><head><meta http-equiv="refresh" content="0; url="'+board.info["link"][0]+'"></head><body>Please wait. redirecting...</body></html>');
#  exit(0);

# -----------------------------------------------------------------------------------------
functionsOnBoard = [];

for pin in pins:
  if pin["name"][0] == 'P':
    pin["name"] = pin["name"][1:];
  for func in pin["functions"]:
    if func in pinutils.CLASSES:
      if not pinutils.CLASSES[func] in functionsOnBoard:
        functionsOnBoard.append(pinutils.CLASSES[func])

#print(json.dumps(functionsOnBoard))

def has_pinb(brd,pin):
  for pinstrip in brd:
    if pinstrip[0]!='_':
      for p in brd[pinstrip]: 
        if p==pin: return True
  return False

def has_pin(pin):
  if hasattr(board, 'boards'):
    for brdnum in range(len(board.boards)):
      if has_pinb(board.boards[brdnum], pin): return True
    return False
  else:
    return has_pinb(board.board, pin)

# -----------------------------------------------------------------------------------------

def dump_pin(brd, pin, pinstrip):

      pinmap = {};
      if '_pinmap' in brd:
        pinmap = brd['_pinmap'];

      if pin in pinmap:
        pin = pinmap[pin];      
      pininfo = pinutils.findpin(pins, pin, False)


      not_five_volt = False
#      print(json.dumps(pininfo))
      if ("csv" in pininfo) and ("IO" in pininfo["csv"]) and  ("Type" in pininfo["csv"]) and (pininfo["csv"]["Type"]=="I/O") and (pininfo["csv"]["IO"]!="FT") : 
         not_five_volt = True
      if "3.3" in pininfo["functions"]: 
         not_five_volt = True

      writeHTML('    <DIV class="'+pinstrip+'pin pin pin'+pin+'">');
      pinHTML = ''
      if pin!="": pinHTML = '     <SPAN class="pinname">'+pin+"</SPAN>";
      pinHTML2 = '';

      if not_five_volt:
        pinHTML2 += '<SPAN class="pinfunction NOT_5V" title="Not 5v Tolerant">3.3v</SPAN>\n';

      if ("_notes" in brd) and (pin in brd["_notes"]):
        pinHTML2 += '<SPAN class="pinfunction NOTE" title="'+brd["_notes"][pin]+'">!</SPAN>\n';

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
            pinfuncs[name] = { 'cls': cls, 'title': "["+pin+"] "+title, 'name': name, 'id': pin+"_"+func, 'func' : func };

      for func in sorted(pinfuncs.items(),key=lambda x: x[1]['cls']):
        pf = func[1]
        url = False
        if pf["cls"] in pinutils.URLS: url = pinutils.URLS[pf["cls"]]
        if pf["func"] in pinutils.URLS: url = pinutils.URLS[pf["func"]]
        
        if url != False: writeHTML('     <A href="'+url+'" class="pinfunctionlink">');
        writeHTML('     <SPAN class="pinfunction '+pf["cls"]+'" title="'+pf["title"]+'" onMouseOver="showTT(\''+pf["id"]+'\')" onMouseOut="hideTT(\''+pf["id"]+'\')">'+pf["name"]+'</SPAN>')
        if url != False: writeHTML('     </A>');
        writeHTML('     <SPAN class="pintooltip" id="'+pf["id"]+'" style="display:none;">'+pf["title"]+'</SPAN>')        
          
      if reverse: writeHTML(pinHTML2+"\n"+pinHTML)
      writeHTML('    </DIV>')    

if not embeddable:
  writeHTML("""<HTML>
 <HEAD>
""");
writeHTML("""  <STYLE>
   .boardcontainer { position: relative; }
   .board { 
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
   .pinfunctionlink { 
      color : black;
      text-decoration: none;
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
   .NOTE { background-color: #F80; }

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


.line {
  height:2px;background-color:red;position:absolute;
}

.line:hover {
  background-color:#FF00FF;
}

""");
writeHTML("  </STYLE>"+'<script src="http://code.jquery.com/jquery-1.11.0.min.js"></script>')
writeHTML("""
  <SCRIPT type="text/javascript"> 
    function showTT(ttid) { 
      var e = document.getElementById(ttid);
      e.style.display = 'block';
    }
    function hideTT(ttid) { 
      var e = document.getElementById(ttid);
      e.style.display = 'none';
    }
    function drawLine(x1, y1, x2, y2, hover) {
      if (x2 < x1) {
        var temp = x1;
        x1 = x2;
        x2 = temp;
        temp = y1;
        y1 = y2;
        y2 = temp;
      }
      var line = $('<div class="line" alt="'+hover+'"></div>').appendTo($("body"));
      var length = Math.sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
      line.css('width', length + "px");
      var angle = Math.atan((y2 - y1) / (x2 - x1));
      line.css('top', y1 + 0.5 * length * Math.sin(angle) + "px");
      line.css('left', x1 - 0.5 * length * (1 - Math.cos(angle)) + "px");
      line.css('-moz-transform', "rotate(" + angle + "rad)");
      line.css('-webkit-transform', "rotate(" + angle + "rad)");
      line.css('-o-transform', "rotate(" + angle + "rad)");
     }
  </SCRIPT>
""")
if not embeddable:
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
  writeHTML('  <P>Like this? Please tell your friends, blog, or <a href="http://www.espruino.com/Order">support us by buying our board</a>!</P>')
  writeHTML('  <H2>Pinout</H2>')



writeHTML("""
  <P>Hover the mouse over a pin function for more information. Clicking in a function will tell you how to use it in Espruino.</P>
  <ul>
    <li><span class="pinfunction DEVICE">Purple</span> boxes show pins that are used for other functionality on the board. You should avoid using these unless you know that the marked device is not used.</li>
    <li><span class="pinfunction NOTE">!</span> boxes contain extra information about the pin. Hover your mouse over them to see it.</li>
    <li><span class="pinfunction NOT_5V">3.3v</span> boxes mark pins that are not 5v tolerant (they only take inputs from 0 - 3.3v, not 0 - 5v).</li>""")
if has_pin("3.3"): writeHTML("""   <li><span class="pinfunction">3.3</span> is a 3.3v output from the on-board Voltage regulator.</li>""")
if has_pin("GND"): writeHTML("""    <li><span class="pinfunction">GND</span> is ground (0v).</li>""")
if has_pin("VBAT"): writeHTML("""    <li><span class="pinfunction">VBAT</span> is the battery voltage output (see <a href="/EspruinoBoard">the Espruino Board Reference</a>).</li>""")
if "ADC" in functionsOnBoard: writeHTML("""    <li><span class="pinfunction ADC">ADC</span> is an <a href="/ADC">Analog to Digital Converter</a> (for reading analog voltages)</li>""");
if "DAC" in functionsOnBoard: writeHTML("""    <li><span class="pinfunction DAC">DAC</span> is a <a href="/DAC">Digital to Analog Converter</a> (for creating analog voltages). This is not available on all boards.</li>""");
if "PWM" in functionsOnBoard: writeHTML("""    <li><span class="pinfunction PWM">PWM</span> is for <a href="/PWM">Pulse Width Modulation</a>. This creates analog voltages from a digital output by sending a series of pulses.</li>""");
if "SPI" in functionsOnBoard: writeHTML("""    <li><span class="pinfunction SPI">SPI</span> is the 3 wire <a href="/SPI">Serial Peripheral Interface</a>.</li>""");
if "USART" in functionsOnBoard: writeHTML("""    <li><span class="pinfunction USART">USART</span> is a 2 wire peripheral for <a href="/USART">Serial Data</a>.</li>""");
if "I2C" in functionsOnBoard: writeHTML("""    <li><span class="pinfunction I2C">I2C</span> is the 2 wire <a href="/I2C">Inter-Integrated Circuit</a> bus.</li>""");
if "CAN" in functionsOnBoard: writeHTML("""    <li><span class="pinfunction CAN">CAN</span> is for the <a href="http://en.wikipedia.org/wiki/CAN_bus">Controller Area Network</a>. It is not supported by Espruino.</li>""")

writeHTML("  </ul>");

def writeBoard(brd, brdnum):
  boardname = "board"
  if brdnum!=0: boardname += str(brdnum+1)

  writeHTML('  <STYLE>')
  for pinstrip in brd:
    if pinstrip[0]!='_':
      writeHTML("  #"+boardname+" #"+pinstrip+" { position: absolute; }")
      writeHTML("  #"+boardname+" ."+pinstrip+"pin { white-space: nowrap; }")
  if "_css" in brd:
    writeHTML(brd["_css"].replace("#board", "#"+boardname));
  writeHTML('  </STYLE>')

  writeHTML('  <DIV id="'+boardname+'container" class="boardcontainer">')
  writeHTML('  <DIV id="'+boardname+'" class="board">')

  usedpins = []
  for pinstrip in brd:
    if pinstrip[0]!='_':
      writeHTML('   <DIV id="'+pinstrip+'">')
      for pin in brd[pinstrip]:  
        usedpins.append(pin)
        dump_pin(brd, pin, pinstrip)
      writeHTML('   </DIV>')    
    
  otherpins=0
  for pinstruct in pins:
    pin = pinstruct["name"]
    if not pin in usedpins: 
      otherpins = otherpins + 1
    
  writeHTML('  </DIV>')
  writeHTML('  </DIV>')

  if otherpins>0 and not ('_hide_not_on_connectors' in brd and brd["_hide_not_on_connectors"]):
    writeHTML('  <DIV id="otherpins">')
    writeHTML('   <H2>Pins not on connectors</H2>')
    for pinstruct in pins:
      pin = pinstruct["name"]        
      if not pin in usedpins:    
        dump_pin(brd, pin, "otherpins")
    writeHTML('  </DIV>')
  writeHTML('  <P></P>')

if hasattr(board, 'boards'):
  for brdnum in range(len(board.boards)):
    writeBoard(board.boards[brdnum], brdnum)
else:
  writeBoard(board.board, 0)

#writeHTML('<SCRIPT type="text/javascript"> $(function() {');
#writeHTML('var x = $("#board").offset().left+500;');
#writeHTML('var y = $("#board").offset().top+200;');
#d = 12
#writeHTML('drawLine(x+'+str(-5*d)+',y+'+str(-5*d)+',x+'+str(5*d)+',y+'+str(-5*d)+');');
#writeHTML('drawLine(x+'+str(5*d)+',y+'+str(-5*d)+',x+'+str(5*d)+',y+'+str(5*d)+');');
#writeHTML('drawLine(x+'+str(5*d)+',y+'+str(5*d)+',x+'+str(-5*d)+',y+'+str(5*d)+');');
#writeHTML('drawLine(x+'+str(-5*d)+',y+'+str(5*d)+',x+'+str(-5*d)+',y+'+str(-5*d)+');');
#writeHTML('var p;');
#for pinstrip in board.board:
#  if pinstrip[0]!='_':
#    for pin in board.board[pinstrip]:       
#      if pin in pinmap:
#        pin = pinmap[pin];      
#      pininfo = pinutils.findpin(pins, pin, False)
#      if "UQFN48" in pininfo["csv"]:
#        n = int(pininfo["csv"]["UQFN48"])-1        
#        n = (n+12) % 48
#        if n<12:
#          px = (n-6)*d
#          py = -6*d
#        elif n<24:
#          px = 6*d
#          py = ((n-12)-6)*d
#        elif n<36:
#          px = (6-(n-24))*d
#          py = 6*d
#        else:
#          px = -6*d
#          py = (6-(n-36))*d
#
#        writeHTML("p=$('.pinname:contains(\""+pin+".\")');");
#        pinx = "p.offset().left+p.width()/2";
#        piny = "p.offset().top+p.height()/2";
#        writeHTML('drawLine(x+'+str(px)+',y+'+str(py)+','+pinx+','+piny+', "'+pin+'");');
#writeHTML('});</SCRIPT>');

if not embeddable:
  writeHTML(" </BODY>")
  writeHTML("</HTML>")
