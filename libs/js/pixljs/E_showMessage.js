(function(msg,title) {
  g.clear(1);
  g.setFont("4x6");
  g.setFontAlign(0,0);
  var W = g.getWidth();
  var H = g.getHeight();
  if (title) {
    g.drawString(title,W/2,6);
    var w = (g.stringWidth(title)+8)/2;
    g.fillRect((W/2)-w,10,(W/2)+w,10);
  }
  var lines = msg.split("\n");
  var offset = (H + 10 - lines.length*7)/2;
  lines.forEach((line,y)=>
    g.drawString(line,W/2,offset + y*7));
  g.flip();
})