//https://github.com/espruino/Espruino/issues/2197

class Thing { constructor() { this.prop1 = 10; } method1() { console.log("here!"); result = 1;} }
//trace();
var foo = new Thing();
//trace();
console.log(foo.prop1);
foo.method1();

