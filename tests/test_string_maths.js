// maths with strings - https://github.com/espruino/Espruino/issues/90

var a = [
"1234"*1, 
"1234"/1, 
"1234"-0, 
];

var r=1;
a.forEach(function (a) { if (a!=1234) r=0; });
result=r;
