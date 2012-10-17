// Inheritance of all to Object

Object.prototype.x = function() { return 42; }
String.prototype.y = function() { return 43; }
var s = "iuiuyiu";
var sx = s.x();
var sy = s.y();
result = sx == 42 && sy == 43;
