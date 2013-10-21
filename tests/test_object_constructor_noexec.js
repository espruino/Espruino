// Test a constructor when we're not executing

var a = 42;
function Foo() {}
if (false) a = new Foo();
if (false) a = new Foo; 

result = a==42;
