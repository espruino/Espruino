// Objects can have fields named after built-in functions (recent regression) 

var a = { print : function() { return 42; } };

result = a.print()==42;

