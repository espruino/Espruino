#!/usr/bin/node
/** This scans a source code listing file for Thumb assembly
  code instructions that are repeated. It scores them according
  to `no_of_instructions * times_repeated` and outputs the top
  offenders.

  NOT useful for speed-optimised code - only size-optimised with
  `-Os`. Maybe certain function calls are always chained and could
  be merged together - it should find those and tell you roughly
  how many bytes get saved (score * 2)
 */ 

var IGNORE_REGISTERS = true;
var MIN_OCCURANCES = 3;
var MIN_SCORE = 20;


if (process.argv.length!=3) {
  console.error("USAGE: find_common_code espruino_build.lst");
  process.exit(1);
}

var lstFile = process.argv[2];
console.log("Loading "+lstFile);
var lst = require("fs").readFileSync(lstFile).toString().split("\n");



function getAssembly(line) {
  line = line.trim();
  if (line.indexOf("\t")<0) line="";
  line = line.substr(line.indexOf("\t")+1);
  if (line.indexOf("\t")<0) line="";      
  line = line.substr(line.indexOf("\t")+1);

  
  if (IGNORE_REGISTERS)
    line = line.replace(/r[0-9]/g,"rX");

  return line;
}

function scanLines(lst) {
  var occurances = {};
  var mapCodeToIdx = {};
  var mapIdxToCode = [];
  var lineUses = [];
  var history = [];
  console.log("Scanning for use count");
  lst.forEach(function(line) {
    line = getAssembly(line);

    if (line!="") {
      var lineIdx = mapCodeToIdx[line];
      if (lineIdx===undefined) {
        lineIdx = mapCodeToIdx[line] = mapIdxToCode.push(line)-1;
        lineUses.push(1);
      } else {
        lineUses[lineIdx]++;
      }
    }
  });
  console.log("Scanning for history");
  lst.forEach(function(line) {
    line = getAssembly(line);

    if (line=="") history = [];
    else {
      var lineIdx = mapCodeToIdx[line];
      if (lineUses[lineIdx]<MIN_OCCURANCES) {
        history = [];        
      } else {
        history.push(lineIdx);
        for (var l=0;l<history.length-1;l++) {
          var text = history.slice(l).join(",");
          if (occurances[text]===undefined)
            occurances[text]=1;
          else
            occurances[text]++;
        }
      }
    }
  });
  console.log("Packing");
  var arr = [];
  for (var k in occurances) {
    if (occurances[k]<MIN_OCCURANCES) continue; // nope
    var l = k.split(",").length;
    // score is how much we could save if we put it in a function
    var score = (l-1)*occurances[k];  
    if (score < MIN_SCORE) continue; // not worth listing
    
    arr.push([score,occurances[k],l,k]);
  }
  console.log("Sorting");        
  arr.sort(function(a,b) {
    var ai = a[0];
    var bi = b[0];
    return (ai==bi)?0:( (ai<bi)?1:-1 );
  });
  console.log("Repacking");
  arr.forEach(function(a) {
    var code = [];
    a[3].split(",").forEach(function(c) { code.push(mapIdxToCode[c]); });
    
    console.log("Score ------------- "+a[0]+" ("+a[1]+" uses)");
    code.forEach(function(s) { console.log("  "+s);});
  });
}

scanLines(lst);
