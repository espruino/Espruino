function a(a,b,c) { return a+b+c; }
var v = a.apply(undefined,["hi ","there"," world"]);

result = v == "hi there world";
