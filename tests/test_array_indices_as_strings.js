// Array indices are stored numerically, however in many cases when iterating they need to be returned to code as strings!

function checkArray(a) {
  console.log(Object.keys(a));
  for (var i in a) {
    if ("string"!= typeof i)
      throw new Error("FOR...IN: Got array key type "+typeof i);
  }
  Object.keys(a).forEach(i => {
    if ("string"!= typeof i)
      throw new Error("OBJECT.KEYS: Got array key type "+typeof i);
  });
  Object.getOwnPropertyNames(a).forEach(i => {
    if ("string"!= typeof i)
      throw new Error("OBJECT.KEYS: Got array key type "+typeof i);
  });
}

a = [1,2,3,4,5];
checkArray(a);

a = [];
a[2] = 5;
a["4"] = 6;
a["0402"] = 7;
a["test"] = 7;
checkArray(a);

result=1; // if we had an exception we won't get here
