var tests = 0;
var testsPass = 0;

function test(item, value) {
  tests++;
  if (eval(item) !== value) {
    console.log('ERROR:', item, 'should be', value, 'but found', '' + eval(item));    
  } else testsPass++;
}

test("({}).toString()","[object Object]");
test('({toString:function() { return "Hello"; }}).toString()', "Hello");
test('({toString:function() { return "Hello"; }})+5',"Hello5");


result = testsPass == tests;
