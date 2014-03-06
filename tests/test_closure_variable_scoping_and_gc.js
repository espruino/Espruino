// Function call variable scoping
// also garbage collection as scope will cause circular link

function foo() {
 var r = 40;
 return  function(x) { return r+x; };
}

result = foo()(2) == 42;
