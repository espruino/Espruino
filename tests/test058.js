// Object stuff

result = 0;
Array.prototype.fail = function() {  }
var c = new Array();
JSON.stringify(c); // this failed??
Array.prototype.win = function() { result=1; }

c.win();

