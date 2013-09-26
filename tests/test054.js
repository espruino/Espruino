// test setTimeout with objects

var a = { cmd : "result=1;", w : "world", foo: function() { print('hello '+this.w); eval(this.cmd); } }; 

print("1x intentional error ------");
setTimeout(a,100);
print("---------------------------");   
// shouldn't work - but used to assert fail!

//setTimeout("a.foo()",100); // works
setTimeout(a.foo,100);

