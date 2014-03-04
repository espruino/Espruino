function foo() {}
setInterval(foo, 10);
changeInterval(foo, 10); // should just warn/error?
clearInterval();
result = 1;
