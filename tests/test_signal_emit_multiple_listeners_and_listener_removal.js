// Test Signal Emit - with two listeners per emit, and some removals!
var val = 1;
var result = false;

function Lala() {
}

var foo = new Lala();

foo.on('up', function() { val+=99; });
foo.removeAllListeners();

foo.on('up', function() { val+=99; });
foo.on('up', function() { val+=3; });
foo.on('down', function(x) { val-=x; });
foo.removeAllListeners('down');
foo.on('down', function(x) { val-=x; });


foo.emit('up');
foo.emit('down', 1+99+3-42);

// set result after a timeout - to allow the events time to execute
setTimeout("result = val==42;",1);