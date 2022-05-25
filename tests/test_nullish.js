// test the nullish coalescing operator

result = true;

result &= null ?? true;
result &= undefined ?? true;

var a = null;
result &= a ?? true;

var b = undefined;
result &= b ?? true;

var c = true;
result &= c ?? true;

var d = 0;
result &= (d ?? 1) === 0;

var e = false;
result &= (e ?? 1) === false;

// Check token is handled correctly by the internal pretokeniser
E.setFlags({pretokenise:1})
function a(a) { return  a??5 }
print (a.toString());
result &= a.toString() == "function (a) {return a ?? 5}";

