
var a = 0;

function foo(b) {
  a = arguments;
}

foo(1,2,3,42);

var r1 = a[0]==1 && a[1]==2 && a[2]==3 && a[3]==42 && a.length==4;

foo.call(undefined,1,2,3,42);

var r2 = a[0]==1 && a[1]==2 && a[2]==3 && a[3]==42 && a.length==4;

result = r1 && r2;


