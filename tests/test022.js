/* Javascript eval */

mystructure = { a:39, b:3, addStuff : function(c,d) { return c+d; }, d:undefined, e: [ undefined ] };

mystring = JSON.stringify(mystructure); 

// 42-tiny-js change begin --->
// in JavaScript eval is not JSON.parse
// use parentheses or JSON.parse instead
//mynewstructure = eval(mystring);
mynewstructure = eval("("+mystring+")");
mynewstructure2 = JSON.parse(mystring);
//<--- 42-tiny-js change end

result = mynewstructure.addStuff(mynewstructure.a, mynewstructure.b) == 42 && mynewstructure2.addStuff(mynewstructure2.a, mynewstructure2.b) == 42;
