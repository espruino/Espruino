/* After relatively recent memory optimisation, functions that start with  'return'
 are stored as special types. Overwriting one with another caused issues. */

function a() {
  return 41;
}
function a() {
  if (1) return 42;
}

function b() {
  if (1) return 10;
}
function b() {
  return 11;
}

result = a()==42 && b()==11;
