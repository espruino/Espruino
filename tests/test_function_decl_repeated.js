// Memory leak when defining functions #359
// https://github.com/espruino/Espruino/issues/359

function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 
function test() {console.log("rawr");} 

var mem;
// use timeout to ensure that the actual source code has been freed
setTimeout(function() {
  mem = process.memory().usage; 
  console.log(mem);
  result = mem < 90;
},10);
