// Typed Array Test

var buf = new ArrayBuffer(4);
var a16 = new Uint16Array(buf);
var a8 = new Uint8Array(buf);

print(a16[0] = 0x1234);

var r = [a8[0], a8[1]];

result = r[0]==0x34 && r[1]==0x12 && a8.length==4 && a16.length==2;
