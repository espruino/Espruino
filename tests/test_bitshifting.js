// test for shift
var a = (2<<2);
var b = (16>>3);
var c = ((-1&0xFFFFFFFF) >>> 16);
var d = -7 >>> 0;

result = a==8 && b==2 && c == 0xFFFF && d == 0xFFFFFFF9;
