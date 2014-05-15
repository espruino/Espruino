result = 0;
var fdr = E.openFile('./tests/FS_API_Test.txt','r');
var fdw = E.openFile('./tests/FS_API_Pipe_Test.txt','w');
//Pipe the contents of FS_API_Test.txt to FS_API_Pipe_Test.txt, async in 8 bytes chunks.
fdr.pipe(fdw, { chunkSize:8, complete:function(pipe) {
    var buffer1 = "";
    var buffer2 = "";
    // attempt to read 200 bytes from the file, starting at position 0;
    pipe.source.read(buffer1,200,0);
    pipe.source.close();
    pipe.destination.close();
    var fd = E.openFile('./tests/FS_API_Pipe_Test.txt','r');
    fd.read(buffer2,200,0);
    fd.close();
    result = (buffer1 == buffer2);
}});

