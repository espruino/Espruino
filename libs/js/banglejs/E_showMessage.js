(function(message,options) {
  if ("string" == typeof options)
    options = { title : options };
  options = options||{};
  var R = Bangle.appRect, Y = R.y, W = R.w;
  g.reset().clearRect(R).setFontAlign(0,0); // clear screen
  var title = g.findFont(options.title||"", {w:W-2,wrap:1,max:24});
  if (title.text)
    g.setColor(g.theme.fgH).setBgColor(g.theme.bgH).
      clearRect(0,Y,W-1,Y+4+title.h).
      drawString(title.text,W/2,Y+4+title.h/2);
  Y += title.h+4;
  var H = R.y2-Y;
  if (options.img) {
    var im = g.imageMetrics(options.img);
    g.drawImage(options.img,(W-im.width)/2, Y + 6);
    H -= im.height;
    Y += im.height;
  }
  if (message !== undefined) {
    var msg = g.findFont(message, {w:W-2,h:H,wrap:1,trim:1,min:16});
    g.setColor(g.theme.fg).setBgColor(g.theme.bg).
      drawString(msg.text,W/2,Y+H/2);
  }
  g.flip(); // force immediate show of message
  Bangle.setLCDPower(1); // ensure screen is on
})