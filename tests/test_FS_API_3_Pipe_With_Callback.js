result = 0;
var fdr = require("fs").open('./tests/FS_API_Test.txt','r');
var fdw = require("fs").open('./tests/FS_API_Pipe_Test.txt','w');
//Pipe the contents of FS_API_Test.txt to FS_API_Pipe_Test.txt, async in 8 bytes chunks.
fdr.pipe(fdw,8, function(pipe) {
    var buffer1 = "";
    var buffer2 = "";
    // attempt to read 200 bytes from the file, starting at position 0;
    pipe.Source.read(buffer1,200,0);
    pipe.Source.close();
    pipe.Destination.close();
    var fd = require("fs").open('./tests/FS_API_Pipe_Test.txt','r');
    fd.read(buffer2,200,0);
    fd.close();
    result = (buffer1 == buffer2);
});

