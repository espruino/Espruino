tests=0;
testPass=0;

function test(a, b) {
  tests++;
  if (a==b) {
    return testPass++;
  }
  console.log("Test "+tests+" failed - ",a,"vs",b);
}

//https://github.com/espruino/Espruino/issues/1889
test("".replace(/x*/g, ""),  '')

test("".replace(/x*/g, "_"), '_')

test("Hello".replace(/x*/g, "_"), '_H_e_l_l_o_')

// Another issue
test(JSON.stringify("".match(/x*/g)), JSON.stringify(['']))

// https://github.com/espruino/Espruino/issues/1888
test("xxx".replace(/x/g, ""), "")


result = tests==testPass;
console.log(result?"Pass":"Fail",":",tests,"tests total");
