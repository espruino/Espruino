// Typed Array Test

var a = new Uint32Array(32);
for (i=0;i<a.length;i++) a[i] = i*2;

total = 0;
for (i=0;i<a.length;i++) total += a[i];

result = total == 992;

