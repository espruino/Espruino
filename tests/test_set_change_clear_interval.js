function foo() {}
setInterval(foo, 10);
var hadError = false;
try {
  changeInterval(foo, 10); // should just warn/error?
} catch (e) {
  print("Caught "+e);
  hadError = true;
}
clearInterval();
result = hadError;
