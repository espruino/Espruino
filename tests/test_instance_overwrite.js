// Instance properties overwrite prototype #360
// https://github.com/espruino/Espruino/issues/360

var r;

function A() {
}
A.prototype.doStuff = function() {
  r = "Great";
}

var b = new A();

var c = new A();
c.doStuff = function() {
  r = "Not great";
}

c.doStuff();

var r1 = r;

b.doStuff(); 

var r2 = r;

var r3 = (delete b.doStuff);

var d = new A();
d.doStuff();

var r4 = r;

result = (r1 == "Not great") && (r2 == "Great") && (r3==false) && (r4 == "Great");
