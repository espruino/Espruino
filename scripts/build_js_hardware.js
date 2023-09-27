#!/usr/bin/env node
/* This builds a js file containing the addresses of a chip's PERIPHERALS and
 the values of the bits in each register. Can be used for quickly building
 ways to access the underlying hardware from JS.

This has only been tested with STM32 boards so far
*/

// ===================================================================================
// These files are the source of information for the JS files - change depending on STM32F1/F4 chip
//var INPUTFILE = "targetlibs/stm32f1/lib/stm32f10x.h";
//var DEFINITIONS =  {"STM32F10X_XL":true};
//var INPUTFILE = "targetlibs/stm32f4/lib/stm32f411xe.h";
//var DEFINITIONS =  {"STM32F401xx":true};
var INPUTFILE = "targetlibs/nrf5x_12/components/device/nrf52.h";
var DEFINITIONS =  {"STM32F401xx":true};

// The PERIPHERALS you need to output code for
//var PERIPHERALS = ["RCC","RTC","PWR"];
var PERIPHERALS = [];
var OUTPUT_GHIDRA = "nrf52.pspec";
// ===================================================================================

var fs = require('fs');

var structContents = undefined;
var structSize = 0;
var typeSizes = {
  "uint32_t" : 4,
  "int32_t" : 4,
  "uint16_t" : 2,
  "uint8_t" : 1,
  "CAN_FIFOMailBox_TypeDef" : NaN,
  "CAN_TxMailBox_TypeDef" : NaN,
  "CAN_FilterRegister_TypeDef" : NaN,
};
var ifDefs = [];


var structs = {}; ///< structures representing periphgerals
var periphs = {}; ///< Defined PERIPHERALS { name : ..., type : ..., base : ... } 


var header = fs.readFileSync(INPUTFILE).toString().replace(/(\/\*[^\n*]*)\r\n(.*\*\/)/g,"$1$2");
header = header.replace(/\/\*[^\*]*\*\//g,""); // remove comments

header.split("\n").forEach(function(oline) {
  //console.log(JSON.stringify(oline));
  var line = oline.trim();
  if (line=="") return;
  if (line.indexOf("#ifndef ")==0) {
    ifDefs.push(!(line.substr(8).trim() in DEFINITIONS));
    return;
  } else if (line.indexOf("#ifdef ")==0) {
    ifDefs.push((line.substr(7).trim() in DEFINITIONS));
    return;
  } else if (line.indexOf("#if ")==0) {
    console.warn("Uh-oh, we have a #if - assuming it is true:"+line);
    ifDefs.push(true);
    return;
  } else if (line.indexOf("#else")==0) {
    ifDefs[ifDefs.length-1] = !ifDefs[ifDefs.length-1];
    return;
  } else if (line.indexOf("#endif")==0) {
    ifDefs.pop();
    return;
  }

  for (var i in ifDefs)
    if (!ifDefs[i]) return;

  if (line.indexOf("#error ")==0) {
    console.warn(line);
  }


  if (line.indexOf("#define")==0) {

    line = line.substr(7).trim();
    if (line.includes("*")) {
      var m = line.match(/(\w*)\s*\(\((\w*)\s*\*\)\s*(\w*)\)/);
      if (m) periphs[m[1]] = { name : m[1], type : m[2], base : m[3] };
    } else {
      var i = line.indexOf(" ");
      var dkey = line.substr(0,i);
      var dval = line.substr(i).trim();
      dval = dval.replace(/\(uint32_t[ ]*\)/,"");
      dval = dval.replace(/UL$/,"");
      console.log(dkey,dval);
      DEFINITIONS[dkey] = dval;
    }
  } else if (line=="typedef struct" || line=="typedef struct {") {
    structContents = {};
    structSize = 0;
  } else if (line[0]=="{") {
  } else if (line[0]=="}") {
    if (line.indexOf("_TypeDef;")>=0) {
      var name = line.substring(line.lastIndexOf(" ")+1,line.indexOf("_TypeDef;"));
      structs[name] = structContents;
      typeSizes[name] = structSize;
    }
    if (line.indexOf("_Type;")>=0) {
      var name = line.substring(line.lastIndexOf(" ")+1,line.indexOf("_Type;")+5);
      structs[name] = structContents;
      typeSizes[name] = structSize;
    }
    structContents = undefined;
    structSize = 0;
  } else if (structContents) {
    if (line.substr(0,2)!="/*") {
      if (line.substr(0,2)=="__") line=line.substr(line.indexOf(" ")+1).trim();
      var type = line.substr(0,line.indexOf(" "));
      line = line.substr(line.indexOf(" ")+1).trim();
      var name = line.substr(0,line.indexOf(";"));
      line = line.substr(line.indexOf(";")+1).trim();
      if (!(type in typeSizes)) {
 console.log(typeSizes);
 throw new Error("Unknown type "+type+" in "+JSON.stringify(oline));
}
      var size = typeSizes[type];
      var elSize = size;
      if (name.indexOf("[")>=0) {
        var c = name.substring(name.indexOf("[")+1, name.indexOf("]"));
        size *= c;
      }         
      structContents[name] = { offset : structSize, size : size, elSize : elSize };
      structSize += size;      
    }
  }
});

//console.log(structs)

// find base addresses
var bases = {};
for (var def in DEFINITIONS) {
  if (def.substr(-5)=="_BASE") {
    var v = DEFINITIONS[def].              
              replace(/(\()([A-Z])/,"$1 $2").
              replace(/([A-Z])(\))/,"$1 $2").
              split(" ");
    v = v.map(function(tk) {
      if (tk in bases) return bases[tk];
      return tk;
    });
    bases[def] = eval(v.join("").replace(/UL$/,""));
  }
}

function out(s) {
  console.log(s);
}

//console.log(periphs);

PERIPHERALS.forEach(function(periph) { 
  var base = bases[periph+"_BASE"];
  var data = { a : {}, f : {} };
  var periphType = periph.replace(/[0-9]/g,"");
  var periphStruct = structs[periphType];

  for (var key in periphStruct) {
    if (key.substr(0,8)=="RESERVED") continue;
    // address
    data.a[key] = base+periphStruct[key].offset;
    // flags
    var pref = periph+"_"+key+"_";
    for (var d in DEFINITIONS) {
      if (d.substr(0,pref.length)==pref) {
        if (!data.f[key]) data.f[key] = {};
        try {
          data.f[key][d.substr(pref.length)] = eval(DEFINITIONS[d]);
        } catch (e) {}
      }
    }
  }

  // output
  out("var "+periph+" = "+JSON.stringify(data,null,2)+";");
});

if (OUTPUT_GHIDRA) {
  console.log("Outputting Ghidra XML -> "+OUTPUT_GHIDRA);
  var xml = `<?xml version="1.0" encoding="UTF-8"?>

<processor_spec>
  <properties>
    <property key="addressesDoNotAppearDirectlyInCode" value="true"/>
    <property key="allowOffcutReferencesToFunctionStarts" value="true"/>
    <property key="useNewFunctionStackAnalysis" value="true"/>
    <property key="emulateInstructionStateModifierClass" value="ghidra.program.emulation.ARMEmulateInstructionStateModifier"/>
  </properties>
  <programcounter register="pc"/>
  <context_data>
    <context_set space="ram" first="0x0000" last="0x7FFFF">
      <set name="TMode" val="1" description="0 for ARM 32-bit, 1 for THUMB 16-bit"/>
      <set name="LRset" val="0" description="0 lr reg not set, 1 for LR set, affects BX as a call"/>
    </context_set>
    <tracked_set space="ram" first="0x1fff0000" last="0x2000ffff">
      <set name="spsr" val="0"/>
    </tracked_set>
    <tracked_set space="ram" first="0x4000000" last="0x4007fffe">
      <set name="spsr" val="0"/>
    </tracked_set>	
	
  </context_data>
  
  <default_symbols>
    <symbol name="MasterStackPointer" address="ram:0x0" entry="false" type="code_ptr"/>
    <symbol name="Reset" address="ram:0x4" entry="true" type="code_ptr"/>
    <symbol name="NMI" address="ram:0x8" entry="true" type="code_ptr"/>
    <symbol name="HardFault" address="ram:0xC" entry="true" type="code_ptr"/>
    <symbol name="MemManage" address="ram:0x10" entry="true" type="code_ptr"/>
    <symbol name="BusFault" address="ram:0x14" entry="true" type="code_ptr"/>
    <symbol name="UsageFault" address="ram:0x18" entry="true" type="code_ptr"/>
    <symbol name="Reserved1" address="ram:0x1c" entry="true" type="code_ptr"/>
    <symbol name="Reserved2" address="ram:0x20" entry="true" type="code_ptr"/>
    <symbol name="Reserved3" address="ram:0x24" entry="true" type="code_ptr"/>
    <symbol name="Reserved4" address="ram:0x28" entry="true" type="code_ptr"/>
    <symbol name="SVCall" address="ram:0x2c" entry="true" type="code_ptr"/>
    
    <symbol name="Reserved5" address="ram:0x30" entry="true" type="code_ptr"/>
    <symbol name="Reserved6" address="ram:0x34" entry="true" type="code_ptr"/>
    
    <symbol name="PendSV" address="ram:0x38" entry="true" type="code_ptr"/>
    <symbol name="SysTick" address="ram:0x3C" entry="true" type="code_ptr"/>`;
  function sym(name,addr) {
    xml += `    <symbol address="ram:0x${addr.toString(16)}" name="${name}"/>\n`;
  }
  Object.keys(periphs).forEach(function(periphName) {
    //console.log(periphName);
    periph = periphs[periphName];
    var periphStruct = structs[periph.type];
    var periphBase = bases[periph.base];
    //console.log(periphStruct, periphBase);
    sym(periphName, periphBase);
    for (var key in periphStruct) {
      if (key.substr(0,8)=="RESERVED") continue;
      // address
      var addr = periphBase+periphStruct[key].offset;
      // flags
      var l = key.match(/([^\[]*)\[(\d+)\]/);
      if (l) {
        var name = l[1];
        var n = 0|l[2];
        for (var a=0;a<n;a++)
          sym(periphName+"_"+name+"_x"+a, addr + periphStruct[key].elSize*a);
      } else {
        sym(periphName+"_"+key, addr); 
      }
      //
    }
  });
  xml += `  </default_symbols>
</processor_spec>\n`;
  
  fs.writeFileSync(OUTPUT_GHIDRA, xml);
}
