var test1, test2;

var p = new Promise(function(res,rej) {
  throw "Bummer 1";
}).then(function(r) {  
  console.log("then");
}).catch(function(e) {  
  console.log("catch ",e);
  test1 = 1;
});


process.on('uncaughtException', function(e) {
  console.log("expected ",e);
  if (e.toString().indexOf("Unhandled promise rejection")>=0)
    test2 = 1;
});

var p = new Promise(function(res,rej) {
  throw "Bummer 2";
}).then(function(r) {
  console.log("then");
}).then(function(r) {    
  console.log("then");
}).then(function(r) {  
  console.log("then");
}).then(function(r) {  
  console.log("then");
});

setTimeout(function() { 
  result = test1 && test2;
}, 10);

