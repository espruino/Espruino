// https://github.com/espruino/Espruino/issues/1433

process.on('uncaughtException', function(e) {
  print(e,e.stack?"\n"+e.stack:"")
  result = 0;
});

var p = new Promise(function(resolve, reject) {
  setTimeout(function() {
    //trace(p);
    resolve();
  }, 100);
  setTimeout(function() {
    //trace(p);
    reject('B');
  }, 200);
});
p.then(function() {
  console.log('resolved');
  result = 1;
}).catch(function() {
  console.log('rejected');
  result = 0; // shouldn't reject after initial resolve
});


