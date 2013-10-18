// integer string indices - https://github.com/espruino/Espruino/issues/19

var a = [];
var res = [];

a["0"] = 4;
res.push(a[0]==4);
a["000"] = 6;
res.push(a[0]==4);
a["12"] = 5;
res.push(a[12]==5);
a["12"] = 5;
res.push(a[12]==5);
a[34] = 7;
res.push(a["34"]==7);

var r=1;
res.forEach(function (a) { if (!a) r=0; });
result=r;
