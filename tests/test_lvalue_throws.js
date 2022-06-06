// Test that lvalue cannot be assigned to

var a = 0, b = 0;
try {
  (true ? a : b) = 5;
  result = false;
}catch(e){
  result = true;
}

try {
  (a && b) = 5;
  result = false;
}catch(e){
  result |= true;
}

try {
  (a || b) = 5;
  result = false;
}catch(e){
  result |= true;
}

try {
  (a ?? b) = 5;
  result = false;
}catch(e){
  result |= true;
}