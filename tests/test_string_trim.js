function test(a,b) {
  var ea = eval(a);  
  if (JSON.stringify(ea)!=JSON.stringify(b)) {
    console.log(JSON.stringify(a)+" should be "+JSON.stringify(b)+", got "+JSON.stringify(ea))
    result = 0;
  }
}

result = 1;
test('"Hello World".trim()',"Hello World");
test('" \t\n\rHello World".trim()',"Hello World");
test('"Hello World \t\n\r".trim()',"Hello World");
test('" \t\n\rHello World \t\n\r".trim()',"Hello World");
test('"X".trim()',"X");
test('"   X    ".trim()',"X");
test('"     ".trim()',"");
