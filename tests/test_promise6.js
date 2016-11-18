// Test using `then` on an already-resolved promise
var p = new Promise( function(resolve,reject) { resolve(42); });
setTimeout(function() {
  p.then( function(value) {
    result = value==42;
  });
},1);
