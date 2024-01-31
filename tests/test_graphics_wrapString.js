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
  var ta = E.toJS(a);
  var tb = E.toJS(b);
  if (ta!=tb) {
    console.log("GOT      :"+tb);
    console.log("SHOULD BE:"+ta);
    console.log("================");
    b.forEach(l=>console.log(E.toJS(l), g.stringWidth(l)));
    console.log("================");
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

// wrapping when the area is too small to show even one char
g.clear().setFont("4x6");
lines = g.wrapString("XYZ", 2);
SHOULD_BE(lines, ["X","Y","Z"]);

// wrap a long word to multiple lines
g.clear().setFont("4x6");
lines = g.wrapString("ABCDEFGHIJ", 10);
SHOULD_BE(lines, ["AB","CD","EF","GH","IJ"]);

// word too big for a line - should be split
g.clear().setFont("4x6");
lines = g.wrapString("A very LongWord is not here", 30);
SHOULD_BE(lines, ["A very","LongWor","d is","not","here"]);

// don't wrap on commas if long enough
g.clear().setFont("4x6");
lines = g.wrapString("Hello,world", 100);
SHOULD_BE(lines, ["Hello,world"]);
// wrap on commas
g.clear().setFont("4x6");
lines = g.wrapString("Hello,world", 30);
SHOULD_BE(lines, ["Hello,","world"]);

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

// *Just* a bitmap (was a previous failure)
g.clear().setFont("4x6");
lines = g.wrapString("\0\x12\x12\x81\0\x7F\xFF\xBF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFE\x7F\xFF\x1F\xFF\x8F\xFF\xC7\xF9\xE3\xFE1\xFF\xC0\xFF\xF8\x7F\xFF?\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xDF\xFF\xE0",1000)
SHOULD_BE(lines, ["\0\x12\x12\x81\0\x7F\xFF\xBF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFE\x7F\xFF\x1F\xFF\x8F\xFF\xC7\xF9\xE3\xFE1\xFF\xC0\xFF\xF8\x7F\xFF?\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xDF\xFF\xE0"]);

// 4x 8x8 bitmaps - should be split on lines
g.clear().setFont("4x6");
lines = g.wrapString("\0\x08\x08\1\1\2\3\4\5\6\7\8\0\x08\x08\1\1\2\3\4\5\6\7\8\0\x08\x08\1\1\2\3\4\5\6\7\8\0\x08\x08\1\1\2\3\4\5\6\7\8",12)
SHOULD_BE(lines, ["\0\x08\x08\1\1\2\3\4\5\6\7\8","\0\x08\x08\1\1\2\3\4\5\6\7\8","\0\x08\x08\1\1\2\3\4\5\6\7\8","\0\x08\x08\1\1\2\3\4\5\6\7\8"]);

// UTF8 chars - no wrapping expected
g.clear().setFont("4x6");
lines = g.wrapString("F\u00F6n K\u00FCr B\u00E4r", 200);
SHOULD_BE(lines, ["F\u00F6n K\u00FCr B\u00E4r"]);

// Using wrapstring from Storage
require("Storage").write("x","Compacting...\nTakes approx\n1 minute")
lines = g.wrapString(require("Storage").read("x"), 176)
require("Storage").erase("x")
SHOULD_BE(lines, ["Compacting...","Takes approx","1 minute"]);




result = ok;
