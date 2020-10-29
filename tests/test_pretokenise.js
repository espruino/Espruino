E.setFlags({pretokenise:1});

function hello() {
  throw new Error("Whoops");
}

var error;
var results = [];
try {
  hello();
} catch (e) {
  error = e;
//  console.log(e.stack);
  results.push(e.stack.indexOf("throw new Error") != 0);
}


// https://github.com/espruino/Espruino/issues/1745
// 'get x' is ID followed by ID
results.push((function () {
  let X = {
    _x:42,
    get x () { return this._x }
  };
  return X.x;
})()==42);

// https://github.com/espruino/Espruino/issues/1868
function go() {
  Promise.resolve().then(function() {
    console.log("Ok");
  }).catch(function(e) {
    console.log("ERROR",e);
  });
}
go()
// Uncaught Error: Function "" not found!
// So if this works, it'll fall through without modifying results


result = results.reduce((a,b)=>a&&b,true);
