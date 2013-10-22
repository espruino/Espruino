// JSON shouldn't print stuff like __proto__ and constructor

function A() {} 
var a = new A();

result = JSON.stringify(a)=="{}";
