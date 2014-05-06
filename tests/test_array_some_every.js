// test array filter

var a = [1,2,3,4,5];

var r1 = a.some(function(n) {
  return n < 3;
}) === true;

var r2 = a.some(function(n) {
  return n > 5;
}) === false;

var r3 = a.every(function(n) {
  return n < 3;
}) === false;

var r4 = a.every(function(n) {
  return n > 5;
}) === false;

var r5 = a.every(function(n) {
  return n > 0;
}) === true;

result = r1 && r2 && r3 && r4 && r5;
