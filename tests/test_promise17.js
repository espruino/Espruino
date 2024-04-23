// https://github.com/espruino/Espruino/issues/2227
// test chaining with catch...
var sequence = "";

function delay(ms){
  return new Promise(r => setTimeout(r, ms));
}

var p = delay(1);

p.then(() => 'a').then(x => {sequence+=x;}); //add a after a second
p.then(() => 'b').then(x => {sequence+=x;}); //should add b after a second, but logs a instead

setTimeout(function() {
  result = sequence == "ab";
  console.log(result, sequence);
},10);
