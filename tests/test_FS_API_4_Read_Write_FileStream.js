
var fsr = require("fs").createReadStream('./tests/FS_API_Test.txt');
var fsw = require("fs").createWriteStream('./tests/FS_API_WriteStream_Test.txt');
var buffer1="";
var buffer2="";
fsr.read(buffer1,6,22);
fsw.write(buffer1,6,0);
fsw.close();
fsr.close();
var fsr2 = require("fs").createReadStream('./tests/FS_API_WriteStream_Test.txt');
fsr2.read(buffer2,6,0);
fsr2.close();
result = (buffer1 == "FS API" && buffer1 == buffer2);