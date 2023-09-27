(function(msg,title,icon) {
  // TODO: Widgets?
  g.clear(1).setColor("#305080");
  g.fillArc(-Math.PI*0.285,Math.PI*0.285,96);
  g.fillArc(Math.PI*(1-0.285),Math.PI*1.285,96);
  g.fillRect(41,62,195,62);
  g.fillRect(41,175,195,175);
  g.setColor(-1);
  var loc = require("locale");
  var W = g.getWidth();
  var H = g.getHeight();
  if (title) {
    title = loc.translate(title);
    g.setFontGrotesk16().setFontAlign(0,-1,0).setBgColor("#305080").drawString(E.decodeUTF8(title),119,42).setBgColor(0);
  }
  if (icon) {
    g.setBgColor(cBorderBg).drawImage(icon.img, icon.x, icon.y);
  }
  g.setFontGrotesk20().setFontAlign(0,0,0);
  var lines = msg.split("\n");
  var offset = 11 + (H - lines.length*22)/2;
  lines.forEach((line,y)=>g.drawString(E.decodeUTF8(loc.translate(line)),W/2,offset + y*22));
  g.flip();
})