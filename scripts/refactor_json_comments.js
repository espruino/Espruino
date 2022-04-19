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

function splitJSONBlock(jsonBlock) {
  var text = jsonBlock.trim().slice(6,-2);
  var endText = text.indexOf("\n}");
  if (endText<0) throw new "End bracket not found in: "+jsonBlock;
  return { json : text.substr(0,endText+2), description : text.substr(endText+2) };
}

function warn(json, msg) {
  console.log("WARN: "+msg+" in: \n"+JSON.stringify(json,null,2));
}

function error(json, msg) {
  console.log("ERROR: "+msg+" in: \n"+JSON.stringify(json,null,2));
  process.exit(1);
}

function checkField(json, field) {
  if (!json[field]) error(json, "No '"+field+"' field");
}

function refactorJSON(json) {
  var expectedFields = ["type", "#if", "ifdef", "ifndef", "check"];
  var copyFields = ["check"];
  if (!json["type"]) error(json, "No 'type' field");
  var newJSON;

  switch (json["type"]) {
    case "init":
    case "idle":
    case "kill":
        checkField(json, "generate");
        copyFields.push("generate");
        newJSON = {
          type : json["type"]
        };
        break;
    case "include":
        checkField(json, "include");
        copyFields.push("include");
        newJSON = {
          type : "include",
        };
        break;
    case "library":
        checkField(json, "class");
        expectedFields.push("class");
        newJSON = {
          type : "library",
          name : json["class"]
        };
        break;
    case "class":
        checkField(json, "class");
        expectedFields.push("class","memberOf", "prototype","library");
        copyFields.push("not_real_object"); // do we really want this??
        newJSON = {
          type : "object",
          name : json["class"]          
        };
        if (json["memberOf"])
          newJSON["memberOf"] = json["memberOf"];
        if (json["library"])
          newJSON["memberOf"] = json["library"];
        if (json["prototype"])
          newJSON["instanceOf"] = json["prototype"];
        break;
    case "object":
        checkField(json, "name");
        checkField(json, "instanceof");
        expectedFields.push("name","instanceof","memberOf");
        newJSON = {
          type : "object",
          name : json["name"],
          instanceOf : json["instanceof"]                    
        };
        if (json["memberOf"])
          newJSON["memberOf"] = json["memberOf"];
        break;
    case "event":
        checkField(json, "class");
        checkField(json, "name");
        expectedFields.push("class","name");
        copyFields.push("params");
        newJSON = {
          type : "event",
          name : json["name"],
          memberOf : json["class"]
        };
        break;

    case "constructor":
        checkField(json, "class");
        checkField(json, "name");
        if (json["name"] != json["class"]) error(json, "Constructor name/class don't match");
        expectedFields.push("class","name");
        copyFields.push("generate","generate_full","params","return","return_object");
        newJSON = {
          type : "constructor",
          name : json["name"]
        };
        break;

    case "function":
        checkField(json, "name");
        expectedFields.push("name","memberOf");
        copyFields.push("generate","generate_full","params","return","return_object");
        newJSON = {
          type : "function",
          name : json["name"],
          memberOf : "global",
          thisParam : false,
        };
        if (json["memberOf"] && json["memberOf"]!="global") error(json, "Function with non-global memberOf");
        break;
    case "staticmethod":
        checkField(json, "class");
        checkField(json, "name");
        expectedFields.push("class","name");
        copyFields.push("generate","generate_full","params","return","return_object");
        newJSON = {
          type : "function",
          name : json["name"],
          memberOf : json["class"],
          thisParam : false,
        };
        break;
    case "method":
        checkField(json, "class");
        checkField(json, "name");
        expectedFields.push("class","name");
        copyFields.push("generate","generate_full","params","return","return_object");
        newJSON = {
          type : "function",
          name : json["name"],
          memberOf : json["class"]+".prototype",
          thisParam : true,
        };
        break;

    case "variable":
        checkField(json, "name");
        expectedFields.push("name");
        copyFields.push("generate","generate_full","return","return_object");
        newJSON = {
          type : "variable",
          name : json["name"],
          memberOf : "global",
          thisParam : false,
        };
        if (json["memberOf"]) error(json, "Function with memberOf");
        break;
    case "staticproperty":
        checkField(json, "class");
        checkField(json, "name");
        expectedFields.push("class","name");
        copyFields.push("generate","generate_full","return","return_object");
        newJSON = {
          type : "variable",
          name : json["name"],
          memberOf : json["class"],
          thisParam : false,
        };
        break;
    case "property":
        checkField(json, "class");
        checkField(json, "name");
        expectedFields.push("class","name");
        copyFields.push("generate","generate_full","return","return_object");
        newJSON = {
          type : "variable",
          name : json["name"],
          memberOf : json["class"]+".prototype",
          thisParam : true,
        };
        break;
    default:
        error(json, "Unknown 'type' field "+JSON.stringify(json["type"]));
  }

  for (key in json) {
    if (copyFields.indexOf(key)>=0) newJSON[key] = json[key];
    else if (expectedFields.indexOf(key)<0) warn(json, "Unexpected field "+JSON.stringify(key));
  }

  if (json["#if"]) {
    newJSON["if"] = json["#if"];
  }
  if (json["ifdef"]) {
    if (newJSON["if"]) error(json, "IF already defined");
    newJSON["if"] = "defined("+json["ifdef"]+")";
  }
  if (json["ifndef"]) {
    if (newJSON["if"]) error(json, "IF already defined");
    newJSON["if"] = "!defined("+json["ifndef"]+")";
  }
   
  if (!newJSON) error(json, "No new JSON");     
  return newJSON;
}

function refactorFile(filename) {
  console.log("Refactoring "+filename);
  var contents = fs.readFileSync(filename).toString();
  var jsonBlocks = contents.match( /\/\*JSON(?:(?!\*\/).|[\n\r])*\*\//g );
  if (jsonBlocks) jsonBlocks.forEach(function(jsonBlock) {
    // parse it
    var block = splitJSONBlock(jsonBlock);
    var json = JSON.parse(block.json); 
    json = refactorJSON(json);
     // now reconstruct
    var newBlock = "/*JSON" + formatJSON(json);
    if (block.description!==undefined)
      newBlock += "\n" + block.description.trim() + "\n";
    newBlock += "*/";
    
   // console.log(newBlock);

    // nasty:
    contents = contents.replace(jsonBlock, newBlock);
  });
  fs.writeFileSync(filename, contents);
}


common.getWrapperFiles(function(files) {
  files.forEach(refactorFile);
});

//refactorFile("src/jswrap_string.c");

