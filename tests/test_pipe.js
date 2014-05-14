var fsr = new File('test.txt','r'); 
var fsw = new File('test_out.txt','w'); 
fsr.pipe(fsw,8,function(pipe) {
    pipe.Source.close();
    pipe.Destination.close();

    result = require('fs').readFileSync('test.txt') == require('fs').readFileSync('test_out.txt');
});
