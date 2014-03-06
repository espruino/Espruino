// test for defining and executing a function

/*function foo(x) { 
 var arr = ["zero","one","two","three"];
 return arr[x];
}*/

var foo = function() {      
 var arr = ["zero","one","two","three"];
 return function (x) { return arr[x]; }  
}(); 

result = foo(1)=="one";
