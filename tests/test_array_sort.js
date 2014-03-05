var a = [5,6,8,1,4,7,7,6,1].sort().toString();
var b = (new Uint8Array([1,1,4,5,6,6,7,7,8])).sort().toString();
var c = [5,6,8,1,4,7,7,6,1].sort(function(a,b) { return (a>b)?-1:((a<b)?1:0); }).toString();


result = "1,1,4,5,6,6,7,7,8"==a && "1,1,4,5,6,6,7,7,8"==b && "8,7,7,6,6,5,4,1,1"==c;

