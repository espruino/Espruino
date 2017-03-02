var testsPass = 0;
var testsRun = 0;

function test(a,b) {
  testsRun++;
  var r = eval(a);
  if (r!=eval(b)) {  
    console.log(a+" ==  "+r+"  expected  "+b);
  } else {
    testsPass++;
  }
}

var ua = new Uint8Array([1,2,3,4,5,6,7,8]);
var ab = ua.buffer;
var d = new DataView(ab);

console.log(ab);

test("d.getInt8(0)", "0x01");
test("d.getInt8(1)", "0x02");
test("d.getInt8(2)", "0x03");
test("d.getInt16()", "0x0102");
test("d.getInt32()", "0x01020304");
test("d.getUint8()", "0x01");
test("d.getUint16()", "0x0102");
test("d.getUint32()", "0x01020304");

test("d.getInt8(0,true)", "0x01");
test("d.getInt16(0,true)", "0x0201");
test("d.getInt32(0,true)", "0x04030201");
test("d.getUint8(0,true)", "0x01");
test("d.getUint16(0,true)", "0x0201");
test("d.getUint32(0,true)", "0x04030201");

d.setFloat32(0,42,true)
test("d.getFloat32(0,true)", "42");
test("d.getUint32()", "10306");
d.setFloat64(0,42.42)
test("d.getFloat64(0)", "42.42");


d.setUint32(0,0x12345678,true)
test("d.getUint32(0,true)", "0x12345678");
d.setUint32(0,0x12345678)
test("d.getUint32(0)", "0x12345678");

result = testsRun==testsPass;
