
var fsr = new File('./tests/FS_API_Test.txt', "r");
var fsw = new File('./tests/FS_API_WriteStream_Test.txt', "w");
var buffer1="";
var buffer2="";
fsr.read(buffer1,6,22);
fsw.write(buffer1,6,0);
fsw.close();
fsr.close();
var fsr2 = new File('./tests/FS_API_WriteStream_Test.txt', "r");
fsr2.read(buffer2,6,0);
fsr2.close();
result = (buffer1 == "FS API" && buffer1 == buffer2);
