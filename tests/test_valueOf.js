// Tests for JS valueOf
// https://github.com/espruino/Espruino/issues/2676
var NULL = ({valueOf:()=>null});
var ZERO = ({valueOf:()=>0});
var ONE = ({valueOf:()=>1});

function test(a,b) {
//  console.log(a);
  var ea = eval(a);  
  if (JSON.stringify(ea)!=JSON.stringify(b)) {
    console.log(JSON.stringify(a)+" should be "+JSON.stringify(b)+", got "+JSON.stringify(ea))
    result = 0;
  }
}

result = 1;
// unary fun
test("!ONE", false);
test("!ZERO", false); // yay JS!
test("+ONE", 1);  // used to fail
test("+ZERO", 0); // used to fail
test("-ONE", -1);
test("-ZERO", 0);
test("~ONE", -2); // used to fail
test("~ZERO", -1);
// binary expr maths
test("0+ZERO", 0);
test("ZERO+0", 0);
test("0+ONE", 1);
test("ONE+0", 1);
test("0-ONE", -1);
test("ONE-0", 1);
// short-circuit ops
test("ZERO||'ok'", ZERO); // used to fail
test("ONE||'ok'", ONE);  // used to fail
test("false||ZERO", ZERO);
test("false||ONE", ONE);
test("true&&ONE", ONE);
test("true&&ZERO", ZERO);
test("ONE&&'ok'", 'ok');
test("ZERO&&'ok'", 'ok'); // used to fail
// nullish
test("NULL??'ok'", NULL); // used to fail
test("ZERO??'ok'", ZERO); // used to fail
test("ONE??'ok'", ONE); // used to fail
// instanceof
test("ONE instanceof Object", true);
// in
test("ONE in [1,2,3]", false); // used to fail
test("ONE in [2,3,4]", false); // used to fail

// ternary
test("ZERO?1:0", 0); // used to fail
test("ONE?1:0", 1); 

