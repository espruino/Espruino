// http://forum.espruino.com/conversations/1194/

var a={};                                                                                             
a["0"]=2;
a[5.3]=2;
var b={"0":3};                                                                                             

var r = [
 a.hasOwnProperty(0),
 a.hasOwnProperty(0.0),
 a.hasOwnProperty("0"),
 0 in a,
 0.0 in a,
 "0" in a,
 b.hasOwnProperty(0),
 b.hasOwnProperty(0.0),
 b.hasOwnProperty("0"),
 0 in b,
 0.0 in b,
 "0" in b,
 // just being sure
 !a.hasOwnProperty(1),
 !a.hasOwnProperty(0.1),
 !a.hasOwnProperty("1"),
 !(1 in a),
 !(0.1 in a),
 !("1" in a),
 !b.hasOwnProperty(1),
 !b.hasOwnProperty(0.1),
 !b.hasOwnProperty("1"),
 !(1 in b),
 !(0.1 in b),
 !("1" in b),
 // floats
 5.3 in a,
 "5.3" in a,
 a.hasOwnProperty(5.3),
 a.hasOwnProperty("5.3"),
 !a.hasOwnProperty(5),
];


var pass = 0;
r.forEach(function(res) { if (res) pass++; } );
result = pass == r.length;
