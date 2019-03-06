

var a = [42,1,2,3,4];

var r  = [
  a.shift()==42,
  a.length==4,
  a[0]==1,
  a[3]==4,
  a.unshift(5,6,7,8) == 8,
  a.length==8,
  a[4]==1,
  a[3]==8
];

// used to cause a referenceerror when shift (brokenly) returned a name
a = [0];
a.shift()

var pass = 0;
r.forEach(function(n) { if (n) pass++; });
result = pass==r.length;
