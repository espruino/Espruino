// Object stuff
// http://phrogz.net/JS/classes/OOPinJS.html
function Person(name,age) {
 this.name = name;
 this.age = age;

 // 'privileged'
 this.rename = function(n){ this.name = n; };
}
Person.prototype.toString = function() { return this.name + " is " + this.age; }


var p = new Person("Bob",1);
p.rename("Gordon");
p.age = "a chump";
var res = p.toString();
result = res == "Gordon is a chump";

/*
result = 0;
Array.prototype.fail = function() {  }
var c = new Array();
JSON.stringify(c); // this failed??
Array.prototype.win = function() { result=1; }

c.win();*/

