// currently has wrong number of elements (iterator issue?)

var a = new Uint8Array(2);
a[1] = 1;
var as = JSON.stringify(new Uint8Array(1));
var bs = JSON.stringify(a);
result = as=="new Uint8Array(1)" && bs=="new Uint8Array([0,1])";
