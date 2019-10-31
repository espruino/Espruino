(function(msg,title) {
  g.clear(1);
  g.setFont("6x8",2);
  g.setFontAlign(0,0);
  var W = g.getWidth();
  var H = g.getHeight();
  if (title) {
    g.drawString(title,W/2,16);
    var w = (g.stringWidth(title)+16)/2;
    g.fillRect((W/2)-w,26,(W/2)+w,27);
  }
  var lines = msg.split("\n");
  var offset = (H - lines.length*16)/2;
  lines.forEach((line,y)=>
    g.drawString(line,W/2,offset + y*16));
})
