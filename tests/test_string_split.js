// test for string split
var b = "1,4,7";
var a = b.split(",");

var c = "Hello There".split("There");
var d = "Hello".split(0); // breaks
var e = "Hello".split("");

result = a.length==3 && a[0]==1 && a[1]==4 && a[2]==7 && c.length==2 && c[0]=="Hello " && c[1]=="" && e.toString()=="H,e,l,l,o";
