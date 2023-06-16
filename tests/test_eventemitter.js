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

var val2 = "";
foo.on('a', function() { val2+="b"; });
foo.addListener('a', function() { val2+="c"; }); // same as .on
foo.prependListener('a', function() { val2+="a"; }); // add first
foo.emit("a");

// set result after a timeout - to allow the events time to execute
setTimeout("result = val==42 && val2='abc';",1);
