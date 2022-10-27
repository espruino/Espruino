// Testing optional chaining

var a;

result = a?.b ?? true;
result &= a?.b.c ?? true;
result &= a?.b() ?? true;

a = null;

result &= a?.b ?? true;