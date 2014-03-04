// New string from constructor

var a = new String();
var b = new String("Hello World");
var c = new String("ABC");
String.prototype.count = function() { return this.length; }

result = a=="" && b=="Hello World" && c.count()==3;
