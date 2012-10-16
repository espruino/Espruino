// Inheritance of all to Object

Object.prototype.x = function() { return 42; }
String.prototype.y = function() { return 43; }
var s = "iuiuyiu";
result = s.x() == 42 && s.y == 43;
