/* Javascript eval */

mystructure = { a:39, b:3, addStuff : function(c,d) { return c+d; } };

mystring = JSON.stringify(mystructure, undefined); 

mynewstructure = eval(mystring);

result = mynewstructure.addStuff(mynewstructure.a, mynewstructure.b);
