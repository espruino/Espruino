// https://github.com/espruino/Espruino/issues/2019
a = Promise.resolve("One").then(function(r) {
  print("_",r);
  return "ONE";
});
/*a = new Promise(r => setTimeout(r,0,"One")).then(function(r) {
  print("_",r);
  return "ONE";
});*/
info = "";

setTimeout(function() {
    a.then(function(r) {
      info += "A";
      print("A",r);
      return "Two";
    }).then(function(r) {
      info += "B";
      print("B",r);
      return "Three";
    });
}, 10);

setTimeout(function() {
    result = info=="AB";
}, 20);
