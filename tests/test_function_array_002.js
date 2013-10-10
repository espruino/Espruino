// Test a function returning an array
function a() { return [function () { return [41,42,43]; }] ; };

var v = a()[0]()[1];

result = v==42;
