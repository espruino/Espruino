// test setTimeout with objects

var a = { cmd : "result=1;", w : "world", foo: function() { print('hello '+this.w); eval(this.cmd); } }; 
var hadError = false;

try {
  // intentional error
  setTimeout(a,100);
  // used to assert fail!
} catch (e) {
  print("Caught "+e);
  hasError = true;
}

//setTimeout("a.foo()",100); // works
//setTimeout(a.foo,100); // shouldn't work
setTimeout(function() { a.foo(); },100);


