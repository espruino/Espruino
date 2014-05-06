// test array reduce on normal and on ArrayBuffer

var a = new Array(5);
var b = new Int16Array(5);
for (var i in b) {
  a[i]=i*100;
  b[i]=i*100;
}

var c = [1,2,3,4,5];

//var suma = [].reduce.call(a, print, 0);

var suma = [].reduce.call(a, function(curr,next,index) { return curr+next+index; }, 0);
var sumb = [].reduce.call(b, function(curr,next,index) { return curr+next+index; }, 0);

var sumc_int = c.reduce(function(p, n) { return p + n } );
var sumc_str = c.reduce.call(c, function(p, n) { return p + n }, "0");

result = suma==1010 && sumb==1010 && sumc_int===15 && sumc_str==="012345";
