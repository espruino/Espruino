// https://github.com/espruino/Espruino/issues/1468

var b = new ArrayBuffer(8);
var arr=new Array(8);
var u8 = new Uint8Array(b);
arr.fill(128,3,6);
u8.fill(128,3,6);
console.log(arr.indexOf(128));//return 3
console.log(u8,r=u8.indexOf(128));//return -1
result = r == 3;
