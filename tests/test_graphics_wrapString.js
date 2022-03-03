var g = Graphics.createArrayBuffer(64,16,8);
Graphics.prototype.dump = _=>{
  var s = "";
  var b = new Uint8Array(g.buffer);
  var n = 0;
  for (var y=0;y<g.getHeight();y++) {
    s+="\n";
    for (var x=0;x<g.getWidth();x++) 
      s+=".#"[b[n++]?1:0];
  }
  return s;
}
Graphics.prototype.print = _=>{
  print("`"+g.dump()+"`");
}

var ok = true;
function SHOULD_BE(b,a) {
  a = E.toJS(a);
  b = E.toJS(b);
  if (a!=b) {
    console.log("GOT :"+b+"\nSHOULD BE:"+a+"\n================");
    ok = false;
  }
}

var lines;

// nothing
g.clear().setFont("4x6");
lines = g.wrapString(undefined, 10);
SHOULD_BE(lines, []);

// small
g.clear().setFont("4x6");
lines = g.wrapString("X", 10);
SHOULD_BE(lines, ["X"]);

// too big
g.clear().setFont("4x6");
lines = g.wrapString("ALongWord", 10);
SHOULD_BE(lines, ["ALongWord"]);

// normal wrap
g.clear().setFont("4x6");
lines = g.wrapString("Hello there lots of text here", 64);
SHOULD_BE(lines, ["Hello there lots","of text here"]);
//g.drawString(lines.join("\n"));g.print();

// with a newline
g.clear().setFont("4x6");
lines = g.wrapString("Hello there\nlots of text here", 64);
SHOULD_BE(lines, ["Hello there","lots of text","here"]);

// forcing a blank line
g.clear().setFont("4x6");
lines = g.wrapString("Hello there\n\nlots of text here", 64);
SHOULD_BE(lines, ["Hello there","","lots of text","here"]);

// bigger font
g.clear().setFont("4x6:2");
lines = g.wrapString("Hello there lots of text here", 64);
SHOULD_BE(lines, ["Hello","there","lots of","text","here"]);

// wrap string correctly when an image is inline
var g = Graphics.createArrayBuffer(32,16,8);
g.clear().setFont("4x6");
lines = g.wrapString("Hello \0\7\5\1\x82 D\x17\xC0 a test", 32);
SHOULD_BE(lines, ["Hello \0\7\5\1\x82 D\x17\xC0","a test"]);
//g.drawString(lines.join("\n"));g.print();

// vector font
g.clear().setFont("Vector18");
lines = g.wrapString("This is wrapping text that fills remaining space", 116);
SHOULD_BE(lines, ["This is","wrapping","text that","fills","remaining","space"]);


result = ok;
