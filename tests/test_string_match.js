// test for string match
// string match
var s = "Hello There".match("here");
// RegExp
var c = "Hello There".match(/There/);
var g = "Hello There".match(/[a-z]/g);

result = s[0] == 'here' && s.index = 7 &&
         c[0] == 'There' && c.index = 6 &&
         g.join(',') == 'e,l,l,o,h,e,r,e';
