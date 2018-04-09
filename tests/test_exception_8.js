// separate scopes
// https://github.com/espruino/Espruino/issues/1367

var ex;
var e = 'hi';
try {
  throw new TypeError();
} catch(e) {
  ex = e;
  console.log("Got error as expected");
}
console.log(e);
result = (ex instanceof TypeError) && e=="hi";
