var a = new Function('return 42');
var ra = a()==42;

var adder = new Function('a', 'b', 'return a + b');
var rb = adder(2, 6)==8;

result = ra && rb;
