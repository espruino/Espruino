// read and write both take a number of bytes and position as the second and third arguments.
// 0 is a valid position, so -1 is used to indicate "don't care" or "current position".
var fd = require("fs").open('./tests/FS_API_Write_Test.txt','w');
var buffer1="Testing Write";
fd.write(buffer1,13,0);
fd.close();
var fd = require("fs").open('./tests/FS_API_Write_Test.txt','r');
var buffer2="";
fd.read(buffer2,13,0);
fd.close();
result = (buffer1 == buffer2);
