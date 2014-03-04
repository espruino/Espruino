// Test Signal Emit
var val = 1;
var result = false;

function Lala() {
}

var foo = new Lala();
foo.on('up', function() { val+=99; });
foo.on('down', function(x) { val-=x; });

foo.emit('up');
foo.emit('down', 58);

// set result after a timeout - to allow the events time to execute
setTimeout("result = val==42;",1);