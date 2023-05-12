var x = 50;
function foo(d) {
  x -= d;
  console.log(x, d);
}

var a = {}; // should be EventEmitter, but currently all objects have it

a.on("data", foo);
a.emit("data", 8);
a.removeListener("data", foo);
a.emit("data", 9);

setTimeout(function() {
  result = x==42;
}, 1);
