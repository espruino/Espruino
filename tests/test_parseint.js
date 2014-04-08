// Test parseInt and parseFloat

var a = [
         parseInt("100"), 100,
         parseInt("0x100"), 256,
         parseInt("0b101"), 5,
         parseInt("1010",2), 10,
         parseInt("10",8), 8,
         parseInt("100",16), 256,
         parseInt("0x100",16), 256,
         parseInt("a",16), 10,
         parseInt("A",16), 10,
         parseInt(NaN), NaN,
         parseInt(NaN,16), NaN,
         parseInt(Infinity), NaN,
         parseFloat("1.11"), 1.11,
         parseFloat(".01"), 0.01,
         parseFloat("100."), 100.0,
         parseFloat("1e+1"), 10.0,
         parseFloat("1.e+1"), 10.0,
         parseFloat("1.2e+2"), 120.0,
         parseFloat("1200e-3"), 1.2,
];

var result = 1;
for (var i=0;i<a.length;i+=2)
  if (a[i]!=a[i+1] && !(isNaN(a[i]) && isNaN(a[i+1]))) result = 0;
