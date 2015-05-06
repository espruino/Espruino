// https://github.com/espruino/Espruino/issues/532

setTimeout(function(a,b,c) { 
  result = (a+" "+b+" "+c)=="Hello World Test"
},10,"Hello","World","Test");


