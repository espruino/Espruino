(function(msg,title) {
  g.clear(1); // clear screen
  Bangle.drawWidgets(); // redraw widgets
  g.reset().setFont("6x8",(g.getWidth()>128)?2:1).setFontAlign(0,0);
  var W = g.getWidth(), H = g.getHeight(), FH=g.getFontHeight();
  var titleLines = g.wrapString(title, W-2);
  var msgLines = g.wrapString(msg||"", W-2);
  g.drawString(msgLines.join("\n"),W/2,(H + titleLines.length*FH)/2);  
  if (title)
    g.setColor(g.theme.fgH).setBgColor(g.theme.bgH).
      clearRect(0,24,W-1,28+titleLines.length*FH).
      drawString(titleLines.join("\n"),W/2,26+(titleLines.length*FH/2));
  g.flip();
})
