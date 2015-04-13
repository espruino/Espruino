var a = new Uint8Array([5,6,7,8,9,10,11,12]);
var b = new Uint8Array(8);

E.mapInPlace(a,b, function(x) { return x+1; });
var r1 = b.toString()=="6,7,8,9,10,11,12,13";

E.mapInPlace(a,b, [12,11,10,9,8,7,6,5,4,3,2,1,0]);
var r2 = b.toString()=="7,6,5,4,3,2,1,0";

E.mapInPlace(a,b, new Uint8Array([12,11,10,9,8,7,6,5,4,3,2,1,0]));
var r3 = b.toString()=="7,6,5,4,3,2,1,0";

E.mapInPlace(a,b, function(a) { return a; }, 4);
var r4 = b.toString()=="0,5,0,6,0,7,0,8"; // high nibble, low nibble

E.mapInPlace(a,b, function(a) { return a; }, 1);
var r5 = b.toString()=="0,0,0,0,0,1,0,1"; // 5 in binary

result = r1 && r2 && r3 && r4 && r5;
