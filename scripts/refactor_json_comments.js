#!/usr/bin/node
/* This was designed to be run once over Espruino 
 *  ... hence it has probably already been run.
 * 
 *  It scans over all old-style JSON elements, reads them in,
 *  formats them, and then spits them out with the description after.
 *  
 *  This will hopefully encourage more verbose descriptions!
 */  

var fs = require("fs");
var common = require("./common.js");

function formatJSON(json) {
  var s = "{\n";
  var first = true;
  for (var key in json) {
    if (!first) s += ",\n";
    var val = json[key];
    s += "  \""+key+"\" : ";
    if (key=="return") {
      s += JSON.stringify(val);
    } else if (key=="params") {
      s += "[\n    ";
      val.forEach(function(param) {
        if (param != val[0]) s+=",\n    ";
        s += JSON.stringify(param);
      });
      s += "\n  ]";
    } else {
      s += JSON.stringify(val,null,"  ");
    }
    first = false;
  }
  s += "\n}";
  return s;
}

common.getWrapperFiles(function(files) {
  files.forEach(function(filename) {
    console.log("Refactoring "+filename);
    var contents = fs.readFileSync(filename).toString();
    var jsonBlocks = contents.match( /\/\*JSON(?:(?!\*\/).|[\n\r])*\*\//g );
    if (jsonBlocks) jsonBlocks.forEach(function(jsonBlock) {
   // parse it
      var json = JSON.parse(jsonBlock.trim().slice(6,-2)); 
      var description = json["description"];
      delete json["description"];
      if (Array.isArray(description))
        description = description.join("\n\n");
      // now reconstruct
      var newBlock = "/*JSON" + formatJSON(json);
      if (description!==undefined)
        newBlock += "\n" + description + "\n";
      newBlock += "*/";
      
      contents = contents.replace(jsonBlock, newBlock);
    });
    
    fs.writeFileSync(filename, contents);
  });
});

