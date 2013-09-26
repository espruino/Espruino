/* Javascript eval */

// 42-tiny-js change begin --->
// in JavaScript eval is not JSON.parse
// use parentheses or JSON.parse instead
//myfoo = eval("{ foo: 42 }");
myfoo = eval("("+"{ foo: 42 }"+")");
//<--- 42-tiny-js change end

result = eval("4*10+2")==42 && myfoo.foo==42;
