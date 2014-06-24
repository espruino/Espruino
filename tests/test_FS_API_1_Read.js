// read and write both take a number of bytes and position as the second and third arguments.
// 0 is a valid position, so -1 is used to indicate "don't care" or "current position".
var fd = E.openFile('./tests/FS_API_Test.txt','r');
fd.skip(22);
var buffer = fd.read(6);
fd.close();
result = (buffer == "FS API");
