let repl = eval(process.env.CONSOLE);
let data = "This is our long string of data that we're going to compress down to transfer as a packet. Let's compress more and more and more and see what happens.";
let compressedData = require("heatshrink").compress(data);
console.log(`Compressed ${data.length} => ${compressedData.length}`);
let json = E.toJS({fn:'test.txt',fs:true,c:1,s:data.length});
if (require("fs").statSync("test.txt")!==undefined) require("fs").unlink("test.txt")
repl.inject("\x10\x01\x60"+String.fromCharCode(json.length)+json);
repl.inject("\x10\x01\x80"+String.fromCharCode(compressedData.length)+E.toString(compressedData));
setTimeout(function() {
  let contents = require("fs").readFileSync("test.txt");
  console.log("test.txt", contents);
  require("fs").unlink("test.txt");
  result = contents==data;  
}, 100);
