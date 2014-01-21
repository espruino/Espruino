var tests=0,testPass=0;
function test(a,b) {
 tests++;
 if (isNaN(a)==b) {
   //console.log("Test "+tests+" passed.");
   testPass++;
 } else {
  console.log("Test "+tests+" failed.");
 }
}

test(NaN,true);       // true
test(undefined,true); // true
test({},true);        // true

test(true,false);      // false
test(null,false);      // false
test(37,false);        // false

// strings
test("37",false);      // false: "37" is converted to the number 37 which is not NaN
test("37.37",false);   // false: "37.37" is converted to the number 37.37 which is not NaN
test("",false);        // false: the empty string is converted to 0 which is not NaN
test(" ",false);       // false: a string with spaces is converted to 0 which is not NaN

// dates
//test(new Date(),false);                // false
//test(new Date().toString(),true);     // true

// This is a false positive and the reason why isNaN is not entirely reliable
test("blabla",true)   // true: "blabla" is converted to a number. 
                  // Parsing this as a number fails and returns NaN

result = tests==testPass;
