// This is ES6 spec - ArrayBuffer has its own map, and it returns the same type of array

var a = new Uint8Array([1,2,5,7,89,9,255]);
var b = a.map(function(v) { return v+1; });
var bs = b.join(",");
// note 255->0 as rolled over

result = bs=="2,3,6,8,90,10,0";
