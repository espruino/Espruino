// arrays with non-array stuff in


var a = [0,1];
a["foo"]="lala";
a[2] = 2;
a[3.333] = 3;

var s1 = "";
for (i in a) s1=s1+i;
var s2 = JSON.stringify(a);
var s3 = a.join('-');
var l = a.length;
result = s1=="012foo3.333" && s2=="[0,1,2]" && s3=="0-1-2" && l==3;

