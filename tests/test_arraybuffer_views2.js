// ArrayBuffer views

// Views != Buffers

var ab = new ArrayBuffer(10); // buffer
var av = new Uint8Array(ab); // view
for (var i in av) av[i]=i;

var b = new Uint8Array(av, 5,1); // should copy and ignore 5,1
var c = new Uint8Array(ab, 5,1); // referenced...

av[0] = 42;
av[5] = 43;

var results = [
  av[0] == 42,
  b.length == 10,
  c.length == 1,
  c[0] == 43,
  b[0] == 0,
  b[5] == 5,
];

var passes = 0;
results.forEach(function(v) { if (v) passes++; });
result = results.length == passes;
