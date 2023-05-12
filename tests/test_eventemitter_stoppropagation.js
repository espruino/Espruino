var X = {}; // in Espruino all objects are EventEmitters
var v = "";
X.on('foo', function() { print("A"); v+="A"; })
X.on('foo', function() { print("B"); v+="B"; E.stopEventPropagation(); })
X.on('foo', function() { print("C"); v+="C"; })
X.emit('foo');
X.emit('foo');

try {
  E.stopEventPropagation();
} catch (e) {
  // expected!
  v += "x";
}

// prints A,B but not C
setTimeout(function() {
  result = v=="xABAB";
}, 1);
