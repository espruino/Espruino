var p = new Promise(function(res,rej) {
  setTimeout(res, 1000, "Hello"); 
}).then(function(r) {  
  console.log("resolve ",r); 
});


// Check that promises work even if the function is called immediately
var p = new Promise(function(res,rej) {
  rej("Hello");
}).catch(function(r) {  
  console.log("reject", r); 
});

