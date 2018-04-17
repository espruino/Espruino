#!/bin/node
/* This builds a js file containing the addresses of a chip's peripherals and
 the values of the bits in each register. Can be used for quickly building
 ways to access the underlying hardware from JS.

This has only been tested with STM32 boards so far
*/

// ===================================================================================
// These files are the source of information for the JS files - change depending on STM32F1/F4 chip
//var INPUTFILE = "targetlibs/stm32f1/lib/stm32f10x.h";
//var definitions =  {"STM32F10X_XL":true};
var INPUTFILE = "targetlibs/stm32f4/lib/stm32f411xe.h";
var definitions =  {"STM32F401xx":true};

// The peripherals you need to output code for
//var peripherals = ["RCC","RTC","PWR"];
var peripherals = ["TIM1"];
// ===================================================================================

var fs = require('fs');

var structContents = undefined;
var structSize = 0;
var typeSizes = {
  "uint32_t" : 4,
  "uint16_t" : 2,
  "uint8_t" : 1,
  "CAN_FIFOMailBox_TypeDef" : NaN,
  "CAN_TxMailBox_TypeDef" : NaN,
  "CAN_FilterRegister_TypeDef" : NaN,
};
var ifDefs = [];


var structs = {};


fs.readFileSync(INPUTFILE).toString().replace(/(\/\*[^\n*]*)\r\n(.*\*\/)/g,"$1$2").split("\n").forEach(function(oline) {
  //console.log(JSON.stringify(oline));
  var line = oline.trim();
  if (line=="") return;
  if (line.indexOf("#ifndef ")==0) {
    ifDefs.push(!(line.substr(8).trim() in definitions));
    return;
  } else if (line.indexOf("#ifdef ")==0) {
    ifDefs.push((line.substr(7).trim() in definitions));
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
    var i = line.indexOf(" ");
    var dkey = line.substr(0,i);
    var dval = line.substr(i).trim();
    if (dval.indexOf("/*")>=0) dval = dval.substr(0,dval.indexOf("/*")).trim();
    definitions[dkey] = dval.replace(/\(uint32_t[ ]*\)/,"");
  } else if (line=="typedef struct") {
    structContents = {};
    structSize = 0;
  } else if (line[0]=="{") {
  } else if (line[0]=="}") {
    if (line.indexOf("_TypeDef;")>=0) {
      var name = line.substring(line.lastIndexOf(" ")+1,line.indexOf("_TypeDef;"));
      structs[name] = structContents;
    }
    structContents = undefined;
  } else if (structContents) {
    if (line.substr(0,2)!="/*") {
      if (line.substr(0,2)=="__") line=line.substr(line.indexOf(" ")+1);
      var type = line.substr(0,line.indexOf(" "));
      line = line.substr(line.indexOf(" ")+1).trim();
      var name = line.substr(0,line.indexOf(";"));
      line = line.substr(line.indexOf(";")+1).trim();
      if (!(type in typeSizes)) throw new Error("Unknown type "+type+" in "+JSON.stringify(oline));
      var size = typeSizes[type];

      if (name.indexOf("[")>=0) {
        var c = name.substring(name.indexOf("[")+1, name.indexOf("]"));
        size *= c;
      }         
      structContents[name] = { offset : structSize, size : size };
      structSize += size;      
    }
  }
});

//console.log(structs)

// find base addresses
var bases = {};
for (var def in definitions) {
  if (def.substr(-5)=="_BASE") {
    var v = definitions[def].              
              replace(/(\()([A-Z])/,"$1 $2").
              replace(/([A-Z])(\))/,"$1 $2").
              split(" ");
    v = v.map(function(tk) {
      if (tk in bases) return bases[tk];
      return tk;
    });
    bases[def] = eval(v.join(""));
  }
}

function out(s) {
  console.log(s);
}

peripherals.forEach(function(periph) { 
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
    for (var d in definitions) {
      if (d.substr(0,pref.length)==pref) {
        if (!data.f[key]) data.f[key] = {};
        try {
          data.f[key][d.substr(pref.length)] = eval(definitions[d]);
        } catch (e) {}
      }
    }
  }

  // output
  out("var "+periph+" = "+JSON.stringify(data,null,2)+";");
});
