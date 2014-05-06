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

var mem = process.memory().usage; 
result = mem < 80;
