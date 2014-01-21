var tests=0,testPass=0;
function test(a,b) {
 tests++;
 if (Array.isArray(a)==b) {
   //console.log("Test "+tests+" passed.");
   testPass++;
 } else {
  console.log("Test "+tests+" failed.");
 }
}

Array.isArrayT = function (v) {
  test(v,true);
}
Array.isArrayF = function (v) {
  test(v,false);
}


// all following calls return true
Array.isArrayT([]);
Array.isArrayT([1]);
Array.isArrayT( new Array() );

//Array.isArrayT( Array.prototype ); // Little known fact: Array.prototype itself is an array.
// Oh come on... Is there any good reason for this being an array apart from spec compliance?
// Implementing this differently is going to be hard

// all following calls return false
Array.isArrayF();
Array.isArrayF({});
Array.isArrayF(null);
Array.isArrayF(undefined);
Array.isArrayF(17);
Array.isArrayF("Array");
Array.isArrayF(true);
Array.isArrayF(false);
Array.isArrayF({ __proto__ : Array.prototype });

// GW added
Array.isArrayF(new Uint8Array(3));

result = tests==testPass;
