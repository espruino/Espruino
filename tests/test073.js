// String iterate and array access

var a = "abcd";
var b = "";
for (i in a) b+=i;

result = a[0]=="a" && a[3]=="d" && a[4]==undefined && b=="0123";
