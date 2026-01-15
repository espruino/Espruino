let repl = eval(process.env.CONSOLE);
let data = "hello world 123";
let json = E.toJS({fn:'test.txt',fs:true,s:data.length});
if (require("fs").statSync("test.txt")!==undefined) require("fs").unlink("test.txt")
repl.inject("\x10\x01\x60"+String.fromCharCode(json.length)+json);
repl.inject("\x10\x01\x80"+String.fromCharCode(data.length)+data);
setTimeout(function() {
  let contents = require("fs").readFileSync("test.txt");
  console.log("test.txt", contents);
  require("fs").unlink("test.txt");
  result = contents==data;  
}, 100);
