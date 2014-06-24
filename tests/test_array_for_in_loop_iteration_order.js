// array iteration order

var a = [0,1];
a[4]=4;
a[2]=2;
var s1 = "";
for (i in a) s1=s1+i;
var s2 = JSON.stringify(a);
var s3 = a.join('-');
var l = a.length;
result = s1=="0124" && s2=="[0,1,2,null,4]" && s3=="0-1-2--4" && l==5;

