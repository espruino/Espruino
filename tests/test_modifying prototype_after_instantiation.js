function A() { }
var a = new A();
A.prototype.getAnswer = function() { return 1; }
result=a.getAnswer();
