var p = new Promise(function(res,rej) {
  setTimeout(res, 10, "Hello"); 
}).then(function(r) {  
  console.log("resolve ",r); 
});


// Check that promises work even if the function is called immediately
var p = new Promise(function(res,rej) {
  rej("Hello");
}).catch(function(r) {  
  console.log("reject", r); 
});

var p = new Promise(function(res,rej) {
  setTimeout(res, 10, "Hello"); 
}).then(function(r) {  
  console.log("resolve ",r); 
});

var p = new Promise(function(res,rej) {
  setTimeout(res, 10, "Hello"); 
}).then(function(r) {  
  console.log("resolve 1",r); 
}).then(function(r) {  
  console.log("resolve 2",r); 
});

trace(p);
