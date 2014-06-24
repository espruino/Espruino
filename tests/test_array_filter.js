// test array filter

var a = [2,3,4,5];
var r1 = a.filter(function(x) {return x % 2 === 0;});
var r2 = a.filter(function(x) {return x % 2 !== 0;});

function eq(a,b) {
  for (var i=0;i<2;i++)
    if (a[i]!=b[i]) return false;
  return true;
}

result = eq(r1,[2,4]) && eq(r2,[3,5]);
