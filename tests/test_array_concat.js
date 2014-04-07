// test for array join

function eq(a, b) {
  if (a.length !== b.length) {
    return false;
  }

  for (var i = 0; i < a.length; i++) {
    if (a[i] !== b[i]) {
      return false;
    }
  }
  return true;
}

var actual = [1,2].concat([3,4]);
var expected = [1,2,3,4];

result = eq(actual, expected);