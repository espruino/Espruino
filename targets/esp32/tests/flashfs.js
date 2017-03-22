
// Please note on first read or write - the Flash file system will be formatted if not intialised.
var files = require("fs").readdirSync();
require("fs").writeFileSync("hello world.txt", "This is the way the world ends\nHello World\nnot with a bang but a whimper.\n");
console.log(require("fs").readFileSync("hello world.txt"));

var f = E.openFile("hello world.txt","r");
f.read(100);
