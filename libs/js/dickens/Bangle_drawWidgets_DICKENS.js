(function() {
  if (!global.WIDGETS) return;
  var pad = 4, w = -pad, wd;
  for (wd of WIDGETS) w += wd.width+pad;
  var x = 119-w/2;
  g.reset();
  for (wd of WIDGETS) {
    wd.x = x;
    wd.y = 26;
    x += wd.width+pad;
    wd.draw(wd);
  }
  g.reset();
})