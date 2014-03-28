// Addition to objects, arrays and functions


function Foo() {}
Foo.prototype.toString = function() { return "Hello World"; }

var a = [
         [1,2,3,4]+42, "1,2,3,442",
         [1,2,3,4]+"foo", "1,2,3,4foo",
         {a:50}+42, "[object Object]42",
         {a:50}+"foo", "[object Object]foo",
         function () {}+42, "function () {}42",
         function () {}+"foo", "function () {}foo", 
];

var result = 1;
for (var i=0;i<a.length;i+=2) {
  if (a[i]!=a[i+1]) result = 0;
}
