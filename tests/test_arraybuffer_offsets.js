// ArrayBuffer offsets

var buf = new ArrayBuffer(6) ;
var a = new Int8Array(buf);
var b = new Int8Array(buf, 3);
var c = new Int8Array(buf, 1,1);
a[4] = 5;
b[2] = 6;
c[0] = 42; 
c[1] = 7; // Ignored as out of range

r = ""+a;
l = c.byteLength;

result= r == "0,42,0,0,5,6" && l==1;
