function arrays_equal(a,b) { return !(a<b || b<a); }

var r = [
  Object.keys(function(){}).length==0,
  arrays_equal(Object.keys([1,2,3]), [0,1,2]),
];


var result = 1;
r.map(function(v) { if (v!=true) result=0; });
