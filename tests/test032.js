var Foo = {
 value : function() { return this.x + this.y; }
};

var a = { prototype: Foo, x: 1, y: 2 };
var b = new Foo(); 
b.x = 2;
b.y = 3;

var result1 = a.value();
var result2 = b.value();
result = result1==3 && result2==5;
