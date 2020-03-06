(function(msg,title) {
  g.clear(1).flip(); // clear screen
  Bangle.drawWidgets(); // redraw widgets
  g.reset().setFont("6x8",2).setFontAlign(0,0);
  var loc = require("locale");
  var W = g.getWidth();
  var H = g.getHeight();
  if (title) {
	title = loc.translate(title);
    g.drawString(title,W/2,34);
    var w = (g.stringWidth(title)+16)/2;
    g.fillRect((W/2)-w,44,(W/2)+w,45);
  }
  var lines = msg.split("\n");
  var offset = (H - lines.length*16)/2;
  lines.forEach((line,y)=>g.drawString(loc.translate(line),W/2,offset + y*16));
  g.flip();
})
