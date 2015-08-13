var PI = Math.PI;
var tests = 0;
var testpass = 0;
function test(cmd, exp) {
  tests++;
  if (eval(cmd)!==exp) {
    console.log(cmd + " !== "+exp)
  } else
    testpass++;
}

function foo() {
  x = 42;         // creates the property x on the global object
  var y = 43;     // declares a new variable, y
  myobj = {
    h: 4,
    k: 5
  };

  test("delete x",true);       // returns true  (x is a property of the global object and can be deleted)
  if (global.x!==undefined) { console.log("fail"); testpass--; }
  test("delete y",false);       // returns false (delete doesn't affect variable names)
  if (y!==43) { console.log("fail"); testpass--; }
  test("delete Math.PI",false); // returns false (delete doesn't affect certain predefined properties)
  if (Math.PI!==PI) { console.log("fail"); testpass--; }
  test("delete myobj.h",true); // returns true  (user-defined properties can be deleted)
  if (myobj.h!==undefined) { console.log("fail"); testpass--; }
  test("delete myobj",true);   // returns true  (myobj is a property of the global object, not a variable, so it can be deleted)
  if (global.myobj!==undefined) { console.log("fail"); testpass--; }
}

foo();

result = tests==testpass;
