(function(msg,options) {
  if (!options) options={};
  if (!options.buttons)
    options.buttons = {"Yes":true,"No":false};
  var loc = require("locale");
  var btns = Object.keys(options.buttons);
  var btnPos = [];
  function draw() {
    g.reset().setFont("6x8",2).setFontAlign(0,-1);
    var Y = Bangle.appRect.y;
    var W = g.getWidth(), H = g.getHeight()-Y, FH=g.getFontHeight();
    var titleLines = g.wrapString(options.title, W-2);
    var msgLines = g.wrapString(msg||"", W-2);
    var y = Y + (H + (titleLines.length - msgLines.length)*FH )/2 - 24;
    if (options.img) {
      var im = g.imageMetrics(options.img);
      g.drawImage(options.img,(W-im.width)/2,y - im.height/2);
      y += 4+im.height/2;
    }
    if (titleLines)
      g.setColor(g.theme.fgH).setBgColor(g.theme.bgH).
        clearRect(0,Y,W-1,Y+4+titleLines.length*FH).
        drawString(titleLines.join("\n"),W/2,Y+2);    
    g.setColor(g.theme.fg).setBgColor(g.theme.bg).
      drawString(msgLines.join("\n"),W/2,y);
    y += msgLines.length*FH+32;
    
    var buttonWidths = 0;
    var buttonPadding = 24;
    g.setFontAlign(0,0);
    btns.forEach(btn=>buttonWidths += buttonPadding+g.stringWidth(loc.translate(btn)));
    if (buttonWidths>W) { // if they don't fit, use smaller font
      g.setFont("6x8");
      buttonWidths = 0;
      btns.forEach(btn=>buttonWidths += buttonPadding+g.stringWidth(loc.translate(btn)));
    }
    var x = (W-buttonWidths)/2;
    btns.forEach((btn,idx)=>{
      btn = loc.translate(btn);
      var w = g.stringWidth(btn);
      x += (buttonPadding+w)/2;
      var bw = 6+w/2;
      btnPos.push({x1:x-bw, x2:x+bw,
                   y1:y-24, y2:y+24});
      var poly = [x-bw,y-16,
                  x+bw,y-16,
                  x+bw+4,y-12,
                  x+bw+4,y+12,
                  x+bw,y+16,
                  x-bw,y+16,
                  x-bw-4,y+12,
                  x-bw-4,y-12,
                  x-bw,y-16];
      g.setColor(g.theme.bg2).fillPoly(poly).setColor(g.theme.fg2).drawPoly(poly).drawString(btn,x,y+1);
      x += (buttonPadding+w)/2;
    });
    Bangle.setLCDPower(1); // ensure screen is on
  }
  g.clearRect(Bangle.appRect); // clear screen
  if (!msg) {
    Bangle.setUI(); // remove watches
    return Promise.resolve();
  }
  draw();
  return new Promise(resolve=>{
    Bangle.setUI("touch", e=>{
      btnPos.forEach((b,i)=>{
        if (e.x >= b.x1 && e.x <= b.x2 &&
            e.y >= b.y1 && e.y <= b.y2) {
          E.showPrompt(); // remove
          resolve(options.buttons[btns[i]]);
        }
      });
    });
  });
})
