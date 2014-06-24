// http://forum.espruino.com/conversations/1200
// Object keys as strings in variables

var a = {};
var b = "0";
a[b] = "hello world";

var r = [a[b],a[0],a["0"]];

result = r[0]=="hello world" && r[1]=="hello world" && r[2]=="hello world";
