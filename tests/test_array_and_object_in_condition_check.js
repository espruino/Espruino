// test IF IN syntax

var a = [0,1,2,3];
var b = ["A","B","C"];
var c = { hello : "there" };

var z = 
(1 in a)?0:1 +
(4 in a)?2:0 + 
("C" in b)?0:4 + 
("D" in b)?8:0 +
("hello" in c)?16:0 +
("there" in c)?0:32;

result = z==0;
