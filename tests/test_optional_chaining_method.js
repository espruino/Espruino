// Testing optional chaining method

// when a is undefined
var a;
result = a?.() ?? true;

// when a is a function
a = () => true;
result &= a?.() ?? false;

// when a.b is undefined
a = {};
result &= a.b?.() ?? true;

// when a.b is a function
a = { b: () => true };
result &= a.b?.() ?? false;