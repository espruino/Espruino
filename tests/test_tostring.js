// Test for toString


function Foo() {}
Foo.prototype.toString = function() { return "Hello World"; }

var a = [
         [1,2,3,4], "1,2,3,4",
         "1234", "1234",
         1234, "1234",
         1234.0, "1234", // strange, but true
         1234.56, "1234.56", 
         {a:2}, "[object Object]",
         new Foo(), "Hello World",
         function (b) {c}, "function (b) {c}"
];

var result = 1;
for (var i=0;i<a.length;i+=2) {
  a[i] = a[i].toString();
  if (a[i]!=a[i+1]) result = 0;
}
