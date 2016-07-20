// https://github.com/espruino/Espruino/issues/513

// Note that it's not that simple. The following case doesn't work in Espruino:
// http://stackoverflow.com/questions/9107240/1-evalthis-vs-evalthis-in-javascript


var a={
  a:function() { return eval("this"); },
//  b:function() { return (1,eval)("this"); }
};

var ra = a.a();
//var rb = a.b();

result = ra==a;// && rb==global;
