result = true;
function test(a,b) {
  if (eval(a)!=b) {
    result=false;
    console.log("FAIL: "+a+" should be "+b);
  }
}

test("3 == 3.0",true);
test("3 === 3.0",true);
test("Math.abs(-5) === 5",true);
// test("Math.log(Math.E*Math.E) === 2", true); // not fair to check this one due to FP inaccuracy
test("Math.log(1) === 0",true);

