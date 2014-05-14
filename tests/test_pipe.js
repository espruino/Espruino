var fsr = new File('test.txt','r'); 
var fsw = new File('test_out.txt','w'); 
fsr.pipe(fsw, { chunkSize:8, complete:function(pipe) {
    pipe.source.close();
    pipe.destination.close();

    result = require('fs').readFileSync('test.txt') == require('fs').readFileSync('test_out.txt');
}});
