// JSON shouldn't print stuff like __proto__ and constructor

function A() {}
var a = new A();

result = JSON.stringify(a)=="{}";

function assertEq(got, expected) {
  if (typeof got !== typeof expected)
    throw new Error("mismatch, " + typeof got + " != " + typeof expected);

  if (typeof got === "object")
    // note: doesn't check keys in `got`
    for (var k in expected)
      assertEq(got[k], expected[k]);
  else if (got !== expected)
    throw new Error("mismatch, " + got + " != " + expected);
}

// no exceptions thrown:
assertEq(JSON.parse('{"a": 1}'), {"a": 1});
assertEq(JSON.parse('["4", 5, "six"]'), ["4", 5, "six"]);
assertEq(JSON.parse('["4", 5, "six", {"x": 5}]'), ["4", 5, "six", {"x": 5}]);
assertEq(JSON.parse('""'), "");
assertEq(JSON.parse('5'), 5);
assertEq(JSON.parse('[]'), []);
assertEq(JSON.parse('{}'), {});
