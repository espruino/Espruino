// #322, #325
function test(a,b) {
  if (JSON.stringify(eval(a))!=b)
    console.log(JSON.stringify(a)+" should be "+JSON.stringify(b)+", got "+JSON.stringify(eval(a)))
}

test('Number(".")', "NaN");
test('Number("Infinity")',"Infinity");
test('Number("-Infinity")',"-Infinity");
test('Number([])',"0");
test('Number([5])',"5");
test('Number(true)',"1");
test('Number([true])',"NaN");
test('Number([false])',"NaN");
test('Number("")',"0");
test('Number([""])',"0");

test('Number("10cm")',"NaN"); // putting up with this for now
test('Number("-_-")',"NaN");
test('Number("+5")',"5");
