// Math.round rounds halves towards +Infinity
function test(a,b) {
  var ea = eval(a);
  if (ea!==b || 1/ea!==1/b) {
    console.log(JSON.stringify(a)+" should be "+b+", got "+ea);
    result = 0;
  }
}

result = 1;
test("Math.round(2.5)", 3);
test("Math.round(-2.5)", -2);
test("Math.round(-1.5)", -1);
test("Math.round(-2.6)", -3);
test("1/Math.round(-0.5)", -Infinity);
test("Math.round(0.5-Math.pow(2,-54))", 0);
test("Math.round(4503599627370497)", 4503599627370497);
test("Math.round(1e20)", 1e20);
test("Math.round(-1e20)", -1e20);
