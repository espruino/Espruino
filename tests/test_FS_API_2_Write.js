// read and write both take a number of bytes and position as the second and third arguments.
// 0 is a valid position, so -1 is used to indicate "don't care" or "current position".
var fd = E.openFile('./tests/FS_API_Write_Test.txt','w');
var buffer1 = "Testing Write";
fd.write(buffer1);
fd.close();
var fd = E.openFile('./tests/FS_API_Write_Test.txt','r');
var buffer2 = fd.read(13);
fd.close();
result = (buffer1 == buffer2);
