// https://github.com/espruino/Espruino/issues/1363

var r = 0;

// pass a promise
Promise.resolve(Promise.resolve(32)).then(x => {console.log(x); if (x==32) r|=1;});


// pass  a thenable
var p1 = Promise.resolve({ 
  then: function(onFulfill, onReject) { onFulfill('fulfilled!'); }
});
if (p1 instanceof Promise) r|=2;
 // true, object casted to a Promise

p1.then(function(v) {
    console.log(v); // "fulfilled!"
    if (v=="fulfilled!") r|=4;
  }, function(e) {
    // not called
    r = 0;
});

setTimeout(function() {
  result = r==7;
},10);
