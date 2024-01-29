// Pretokenising of strings

// unencoded
orig = function(){return "Hello\1\2\3world"}  
// pretokenised
a = function(){"ram"return "Hello\1\2\3world"}  
// pretokenised with stuff after and before
b = function(){"ram"return 4+"Hello world"+5}
// test conversion of atob("...")
c = function(){"ram"return atob("SGVsbG8gV29ybGQ="  ) + " test "+42;} // hello world base64 encoded
// test we don't try and decode something more complicated
d = function(){"ram"return atob("SGVsbG8gV29ybGQ="+"");}

//print("Fn code:",E.toJS(c["\xFFcod"]))
//print("Fn response:",E.toJS(c()))
results = [
  orig["\xFFcod"] == "\"Hello\\1\\2\\3world\"",
  a["\xFFcod"] == "\xD1\rHello\1\2\3world",
  a()=="Hello\1\2\3world",
  b["\xFFcod"] == "4+\xD1\vHello world+5",
  b()=="4Hello world5",
  c["\xFFcod"] == "\xD1\vHello World+\xD1\6 test +42;",
  c()=="Hello World test 42",
  d["\xFFcod"] == "atob(\xD1\x10SGVsbG8gV29ybGQ=+\"\");",
  d()=="Hello World"
];
print(results);
result = results.every(a=>a===true);
