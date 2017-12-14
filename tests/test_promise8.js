// https://github.com/espruino/Espruino/issues/989
function test() {
  if (busy) return;
  (new Promise(function(go){go(1)})).then(function(x) {
    console("Ok");
    result = x;
  }).catch(function() {
    console("Fail");
  });
}
var busy = true;
test(); // it used to fail here
var busy = false;
test();
setTimeout("",100);
