// test array map on ArrayBuffer
var  tests=0, passes=0;


var a = new Int16Array(5);
for (var i in a) a[i]=i;

function test(a,b) {
  a=""+a;
  tests++;
  if (a==b) passes++;
  else console.log(a,"!=",b);
}

test([].map.call(a, function(x) { return x + 1; }), "1,2,3,4,5");
test([].map.call("Hello", function(x) { return x + 1; }), "H1,e1,l1,l1,o1");

var sum = 0;
[].forEach.call(a, function(x) { sum+=x; });

result = tests==passes && sum == 10;
