process.on("uncaughtException", function(x) {
  print("We got an expected uncaughtException: ",x); 
  result = x == "Whoa!";
});
setTimeout(function() { throw "Whoa!"; }, 1);
setTimeout(function() {}, 50);

