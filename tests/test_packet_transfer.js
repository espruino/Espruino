let repl = eval(process.env.CONSOLE);
let data = "hello world 123";
let json = E.toJS({fn:'test',s:data.length});
require("Storage").erase("test");
repl.inject("\x10\x01\x60"+String.fromCharCode(json.length)+json);
repl.inject("\x10\x01\x80"+String.fromCharCode(data.length)+data);
setTimeout(function() {
  let contents = require("Storage").read("test");
  console.log("test", contents);
  require("Storage").erase("test");
  result = contents==data;  
}, 100);
