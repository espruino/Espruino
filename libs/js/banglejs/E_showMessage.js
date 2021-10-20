(function(msg,options) {
  if ("string" == typeof options)
    options = { title : options };
  options = options||{};
  g.clear(1); // clear screen
  Bangle.drawWidgets(); // redraw widgets
  g.reset().setFont("6x8",(g.getWidth()>128)?2:1).setFontAlign(0,-1);
  var Y = global.WIDGETS ? 24 : 0;
  var W = g.getWidth(), H = g.getHeight()-Y, FH=g.getFontHeight();
  var titleLines = g.wrapString(options.title, W-2);
  var msgLines = g.wrapString(msg||"", W-2);
  var y = Y + (H + (titleLines.length - msgLines.length)*FH )/2;
  if (options.img) {
    var im = g.imageMetrics(options.img);
    g.drawImage(options.img,(W-im.width)/2,y - im.height/2);
    y += 4+im.height/2;
  }
  g.drawString(msgLines.join("\n"),W/2,y);  
  if (options.title)
    g.setColor(g.theme.fgH).setBgColor(g.theme.bgH).
      clearRect(0,Y,W-1,Y+4+titleLines.length*FH).
      drawString(titleLines.join("\n"),W/2,Y+2);
  Bangle.setLCDPower(1); // ensure screen is on
})