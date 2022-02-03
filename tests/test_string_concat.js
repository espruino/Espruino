
var testFails = 0;
function t(a,b) {
  if (eval(a) != b) {
    console.log("FAIL", a, "!=", b);
    testFails++;
  };
}

let hello = 'Hello, '
let greetList = ['Hello', ' ', 'Venkat', '!']
t(`hello.concat('Kevin', '. Have a nice day.')`, "Hello, Kevin. Have a nice day.");
t(`"".concat.apply("",greetList)`, "Hello Venkat!");
t(`"".concat({})`, "[object Object]")
t(`"".concat([])`, "");
t(`"".concat(null)`, "null");
t(`"".concat(true)`, "true");
t(`"".concat(4, 5)`, "45");

result = !testFails;
