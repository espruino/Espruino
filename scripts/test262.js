#!/bin/node
/*

Runs Test262 Ecmascript tests on Espruino.

This expects that you've cloned https://github.com/tc39/test262 at the same
level as the Espruino directory. 

*/


process.chdir(__dirname);

var testPasses = [];
var testFails = [];
var assertFails = [];
var commonFailures = {};

var wrapBegin = 
'var $HADERROR = false;\n'+
'function $ERROR(txt) {\n'+
'  console.log("ERROR: ",txt);\n'+
'  $HADERROR = true;\n'+
'}\n'+
'\n'+
'function runTestCase(fn) {\n'+
'  $HADERROR = !fn() || $HADERROR;\n'+
'}\n';
var wrapEnd =
'\nif ($HADERROR) throw "FAIL";';

var linesToIgnore = [
"",
"_____                 _",
"|   __|___ ___ ___ _ _|_|___ ___",
"|   __|_ -| . |  _| | | |   | . |",
"|_____|___|  _|_| |___|_|_|_|___|",
"|_| http://espruino.com",
"Uncaught FAIL"];

function getTest(path) {
  return wrapBegin + require("fs").readFileSync(path) + wrapEnd;;
}

function runTest(path, callback) {
  console.log("------------- "+path);
  var test = getTest(path);

  if (test.indexOf("\\u")>=0) {
    console.log("Not run because of Unicode");    
    return callback();
  }
  if (test.indexOf("es6id:")>=0) {
    console.log("Not run because for ES6");    
    return callback();
  }

  var negative;
  if (test.indexOf("/*---")>0) {
    var desc = test.substring(test.indexOf("/*---")+5, test.indexOf("---*/"));
    var neg = desc.indexOf("negative:");
    if (neg>=0) {
      negative = desc.substring(neg+9, desc.indexOf("\n",neg)).trim();
      console.log("Expecting fail with '"+negative+"'");
    }
  }

  require("fs").writeFileSync("test.js", test); 
  var result = require('child_process').exec('../espruino test.js', { timeout : 10 }, function (error, stdout, stderr) {
    require("fs").unlinkSync("test.js"); 
    if (error) {
      (stdout+"\n"+stderr).split("\n").forEach(function(l) {
        if (!error) return;
        l = l.trim();
        if (l.indexOf("Copyright")>=0 || linesToIgnore.indexOf(l)>=0) return;        
        console.log(l);
        if (negative && l.indexOf(negative>=0)) {
          console.log("NEGATIVE PASS");
          error = false;
        }
        if (l.indexOf("ASSERT")==0) assertFails.push([path, l]);        
        if (commonFailures[l]===undefined) commonFailures[l]=1;
        else commonFailures[l]++;
      });      
    } 
    if (!error) {
      console.log("PASS");
      testPasses.push(path);
    } else {
      testFails.push(path);
      if (negative) console.log("FAIL - expecting '"+negative+"'");
      else console.log("FAIL");
    }
    callback();
  });
}

function recurse(dir, callback) {
  var files = require("fs").readdirSync(dir);
  var c = function() {
    if (files.length==0) return callback();
    var f = files.pop();
    if (f===undefined || f=="." || f=="..") return c();
    var path = dir+"/"+f;
    if (require("fs").statSync(path).isDirectory())
      recurse(path, c);
    else
      runTest(path, c);
  };
  c();
}

function test262(dir, callback) {
  recurse(dir+"/language", function() {
//    recurse(dir+"/built-ins", function() {
       callback();
//    });
  });
}

if (process.argv.length==2) {
  test262("../../test262/test", function() {
    console.log("------------------------------------------");
    console.log("Done! "+testPasses.length+" passes, "+testFails.length+" fails");
    console.log("------------------------------------------");
    console.log("Assertion failures:"); 
    assertFails.forEach(function(s) { console.log(s); });
    console.log("------------------------------------------");
    console.log("Common failures:"); 
    var fails = Object.keys(commonFailures);
    fails.sort(function(a,b) { return commonFailures[b]-commonFailures[a]; });
    for (var i=0;i<50 && i<fails.length;i++) {
      console.log(commonFailures[fails[i]]+":"+fails[i]);
    }
  });
} else if (process.argv.length==3) {
  runTest(process.argv[2], function() {
    console.log("Done!");
  });
} else if (process.argv.length==4 && process.argv[2]=="dump") {
  console.log(getTest(process.argv[3]));
} else {
  console.log("Usage:");
  console.log("  node test262.js                 ; run all tests");
  console.log("  node test262.js path/to/test.js ; run single test");
  console.log("  node test262.js dump path/to/test.js ; dump complete test to console");
}
