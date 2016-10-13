// This is intentionally wrong
// see https://github.com/espruino/Espruino/issues/489

var a = new Uint8Array(2);
a[1] = 1;
var as = JSON.stringify(new Uint8Array(1));
var bs = JSON.stringify(a);
result = as=="[0]" && bs=="[0,1]";
