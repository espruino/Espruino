#!/usr/bin/node

if (process.argv.length == 3 && process.argv[2] == "BANGLEJS") {
  var EMULATOR = "banglejs1";
  var DEVICEID = "BANGLEJS";
} else if (process.argv.length == 3 && process.argv[2] == "BANGLEJS2") {
  var EMULATOR = "banglejs2";
  var DEVICEID = "BANGLEJS2";
} else {
  console.log("USAGE:");
  console.log("  factoryTests.js BANGLEJS");
  console.log("    or");
  console.log("  factoryTests.js BANGLEJS2");
  process.exit(1);
}

const TESTS_DIR = __dirname + "/../tests/FactoryApps";

if (!require("fs").existsSync(__dirname + "/../../EspruinoWebIDE")) {
  console.log("You need to:");
  console.log("  git clone https://github.com/espruino/EspruinoWebIDE");
  console.log("At the same level as this project");
  process.exit(1);
}
eval(require("fs").readFileSync(__dirname + "/../../EspruinoWebIDE/emu/emulator_"+EMULATOR+".js").toString());
eval(require("fs").readFileSync(__dirname + "/../../EspruinoWebIDE/emu/emu_"+EMULATOR+".js").toString());
eval(require("fs").readFileSync(__dirname + "/../../EspruinoWebIDE/emu/common.js").toString().replace('console.log("EMSCRIPTEN:"', '//console.log("EMSCRIPTEN:"'));

var Const = {};
var module = undefined;
var Espruino = require(__dirname + "/../../EspruinoAppLoaderCore/lib/espruinotools.js");

/* we factory reset ONCE, get this, then we can use it to reset
state quickly for each new app */
var factoryFlashMemory = new Uint8Array(FLASH_SIZE);
// Log of messages from app
var appLog = "";
// List of apps that errored
var erroredApps = [];

jsRXCallback = function() {};
jsUpdateGfx = function() {};

function ERROR(s) {
  console.error(s);
  process.exit(1);
}

var lastTxt;
function onConsoleOutput(txt) {
  if (txt == "\r" && lastTxt == "\r") return;
  if (txt && !txt.startsWith("=") ) {
    appLog += txt + "\n";
    lastTxt = txt;
  }
}


function runTest(file) {
  let testLog = "";
  flashMemory.set(factoryFlashMemory);
  jsTransmitString("reset()\n");
  console.log(`Load steps from ${file}`);
  var steps = require("fs").readFileSync(TESTS_DIR + '/' + file).toString().split("\n");
  steps.forEach(step => {
    if (!step) {
      return;
    }
//    console.log("run: " , step);
    appLog = "";
    jsTransmitString(step + "\n");
    testLog += appLog;
  });
  if (testLog.replace("Uncaught Storage Updated!", "").indexOf("Uncaught")>=0) {
    erroredApps.push( { id : file, log : testLog } );
  }
}

// wait until loaded...
setTimeout(function() {
  console.log("Loaded...");
  jsInit();
  jsIdle();
  console.log("Factory reset");
  jsTransmitString("Bangle.factoryReset()\n");
  factoryFlashMemory.set(flashMemory);
  console.log("Ready!");
  appLog = "";
  require("fs").readdirSync(TESTS_DIR).forEach(file => file.endsWith(`.${DEVICEID}.txt`) && runTest(file));
  console.log("Finish");
  jsStopIdle();

  if (erroredApps.length) {
    erroredApps.forEach(app => {
      console.log(`::error file=${app.id}::${app.id}`);
      console.log("::group::Log");
      app.log.split("\n").forEach(line => console.log(`\u001b[38;2;255;0;0m${line}`));
      console.log("::endgroup::");
    });
    process.exit(1);
  }
  process.exit(0);
});
