// Typed Array Test

var a = new Uint8Array(4);
a[0] = 4;
a[1] = 5;
a[2] = 6;
a[3] = 256+7; // test overflow



try {
  b = new Uint8Array(a.buffer, 100000);
  // espruino can't use an offset >65536 - ensure we error, #2439
} catch (e) {
  result = (a[0] + a[1] + a[2] + a[3]) == (4+5+6+7);
}