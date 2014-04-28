// http://people.mozilla.org/~jorendorff/es6-draft.html#sec-array.prototype.fill

var r = [];

var a = [0,0,0,0,0,0,0,0,0,0];
a.fill(2);

r.push(a.join(",") == "2,2,2,2,2,2,2,2,2,2");
a.fill(3,5);
r.push(a.join(",") == "2,2,2,2,2,3,3,3,3,3");
a.fill(4,6,7);
r.push(a.join(",") == "2,2,2,2,2,3,4,3,3,3");
a.fill(5,-3);
r.push(a.join(",") == "2,2,2,2,2,3,4,5,5,5");
a.fill(6,-2,-1);
r.push(a.join(",") == "2,2,2,2,2,3,4,5,6,5");

var pass = 0;
r.forEach(function(n) { if (n) pass++; });
result = pass == r.length;
