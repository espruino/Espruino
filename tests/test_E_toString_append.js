// https://github.com/espruino/Espruino/issues/614
// bad news. E.toString could be a flat string?

var t = E.toString([72,101,108,108,111]); // "Hello"
trace(t);
t+=" World"
trace(t);

result = t=="Hello World" && t.length==11;
