// See https://github.com/espruino/Espruino/issues/109
var r1 = 1;
var r2 = 2;
var a = { hi : "Hello", 
 p1 : function() { r1 = this.hi; },
 p2 : function() { r2 = this.hi; }  };

setTimeout(function() { a.p1(); }, 10);
setTimeout(a.p2, 20);
setTimeout(function() {
  result = r1 == "Hello" && r2 === undefined;
}, 50);

