E.setFlags({pretokenise:1});

function hello() {
  throw new Error("Whoops");
}

var error;
try {
  hello();
} catch (e) {
  error = e;
  console.log(e.stack);
  result = e.stack.indexOf("throw new Error");
}
