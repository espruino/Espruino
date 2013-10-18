// maths with strings

var a = [
"1234"*1, 
"1234"/1, 
"1234"-0, 
];

var r=1;
a.forEach(function (a) { if (a!=1234) r=0; });
result=r;
