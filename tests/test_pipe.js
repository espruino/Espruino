var fr = 'tests/test_pipe.js';
var fw = 'tests/test_pipe.tmp'

var fsr = E.openFile(fr,'r'); 
var fsw = E.openFile(fw,'w'); 
fsr.pipe(fsw, { chunkSize:8, complete:function(pipe) {
    pipe.source.close();
    pipe.destination.close();

    result = require('fs').readFileSync(fr) == require('fs').readFileSync(fw);
    require('fs').unlink(fw); 
}});

