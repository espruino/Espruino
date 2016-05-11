var passes = [];

var p = new Promise(function(res,rej) {
  setTimeout(res, 10, "Hello"); 
}).then(function(r) {  
  if (r=="Hello") passes.push("SimpleResolve");
});

// Check that promises work even if the function is called immediately
var p = new Promise(function(res,rej) {
  rej("Hello");
}).catch(function(r) {  
  if (r=="Hello") passes.push("SimpleReject");
}).then(function(r) {  
  passes.push("FAIL");
});

var p = new Promise(function(res,rej) {
  res("Hello"); 
}).then(function(r) {  
  if (r=="Hello") passes.push("InstantResolve");
}).catch(function(r) {  
  passes.push("FAIL");
});

var p = new Promise(function(res,rej) {
  setTimeout(res, 10, "Hello"); 
}).then(function(r) {  
  if (r=="Hello") passes.push("Resolve1");
}).then(function(r) {  
  if (r=="Hello") passes.push("Resolve2");
});


var p = Promise.resolve("Hello").then(function(r) {  
  if (r=="Hello") passes.push("PreResolved");
}).catch(function(r) {  
  passes.push("FAIL");
});

var p = Promise.reject("Hello").catch(function(r) {  
  if (r=="Hello") passes.push("PreRejected");
}).then(function(r) {  
  passes.push("FAIL");
});


var p = Promise.all([new Promise(function(res,rej) {
  setTimeout(res, 10, "A"); 
}), new Promise(function(res,rej) {
  setTimeout(res, 10, "B"); 
})]).then(function(r) {  
  if (r=="A,B") passes.push("ResolveAll");
});

var p = Promise.all([new Promise(function(res,rej) {
  setTimeout(res, 20, "A"); 
}), new Promise(function(res,rej) {
  setTimeout(rej, 10, "Ok"); 
})]).then(function(r) {  
  passes.push("FAIL");
}).catch(function(r) {  
  if (r=="Ok") passes.push("RejectAll");
});

setTimeout(function() {
  result = passes == "SimpleReject,InstantResolve,PreResolved,PreRejected,SimpleResolve,Resolve1,Resolve2,ResolveAll,RejectAll";
  if (!result) console.log(""+passes);
},30);
