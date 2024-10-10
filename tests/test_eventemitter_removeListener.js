var x = 50;
function foo(d) {
  x -= d;
  console.log(x, d);
}

var a = {}; // should be EventEmitter, but currently all objects have it

a.on("data", foo);
a.emit("data", 8);
// we need a delay, because events are processed in the next idle loop
// and if we remove the listener before then, it won't get the event
setTimeout(function() {
  a.removeListener("data", foo);
  a.emit("data", 9);
}, 1);
setTimeout(function() {
  result = x==42;
}, 2);
