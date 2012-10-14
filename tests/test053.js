// test that interrupt works 3

var a = [2,3,4,5];

var r1 = a.map(function(x) {return x*2;});

var o = { z:3 };
var r2 = a.map(function(x) {return x*this.z;},o);

function eq(a,b) {
  for (var i=0;i<4;i++)
    if (a[i]!=b[i]) return false;
  return true;
}

result = eq(r1,[4,6,8,10]) && eq(r2,[6,9,12,15]);
