// Testing proper lvalue support

// test left hand fall through

var a = 5;
var b = 7;
var c = 0;

result = a || b;
result |= !(c && b);
result |= a ?? b;
result |= a ? b : c;


// test right hand side

result |= c || b;
result |= a && b;
result |= null ?? b;
result |= c ? b : a;