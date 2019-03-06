function test() {
  (new Promise(function(resolve,reject){
    reject("Oh No!");
   })).then(function(x) {
    console.log("Ok");
  }).catch(function() {
    console.log("Fail (expected)");
    result=1;
    throw "This used to leak memory!";
  });
}
test(); // it used to fail here
