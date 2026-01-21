(function(message,options) {
  if (!options) options={};
  if (!options.buttons)
    options.buttons = {"Yes":true,"No":false};
  var btns = Object.keys(options.buttons);
  if (btns.length>6) throw new Error(">6 buttons");
  var btnPos;
  function draw(highlightedButton) {
    g.reset().setFontAlign(0,0);
    var R = Bangle.appRect, Y = R.y, W = R.w;
    var title = g.findFont(options.title||"", {w:W-2,wrap:1,max:24});
    if (title.text) {
      g.setColor(g.theme.fgH).setBgColor(g.theme.bgH).
        clearRect(0,Y,W-1,Y+4+title.h).
        drawString(title.text,W/2,Y+4+title.h/2);
      Y += title.h+4;
    } else Y+=4;
    var BX = 0|"0123233"[btns.length],
        BY = Math.ceil(btns.length / BX),
        BW = (W-1)/BX, BH = options.buttonHeight || ((BY>1 || options.img)?40:50);
    var H = R.y2-(Y + BY*BH);
    if (options.img) {
      var im = g.imageMetrics(options.img);
      g.drawImage(options.img,(W-im.width)/2, Y + 6);
      H -= im.height;
      Y += im.height;
    }
    var msg = g.findFont(message, {w:W-2,h:H,wrap:1,trim:1,min:16});
    g.setColor(g.theme.fg).setBgColor(g.theme.bg).
      drawString(msg.text,W/2,Y+H/2);
    btnPos = [];
    btns.forEach((btn,idx)=>{
      var ix=idx%BX,iy=0|(idx/BX),x = ix*BW + 2, y = R.y2-(BY-iy)*BH + 1,
          bw = BW-4, bh = BH-2;
      btnPos.push({x1:x-2, x2:x+BW-2,
                   y1:y, y2:y+BH});
      var btnText = g.findFont(btn, {w:bw-4,h:BH-4,wrap:1});
      let r = 10;
      let isHL = (idx===highlightedButton);
      g.setColor(isHL ? g.theme.fgH : g.theme.fg2);
      g.fillRect({x:x, y:y, w:bw, h:bh, r:r});
      g.setColor(isHL ? g.theme.bgH : g.theme.bg2);
      g.fillRect({x:x+1, y:y+1, w:bw-2, h:bh-2, r:r-1});
      g.setFontAlign(0,0);
      g.setColor(isHL ? g.theme.fgH : g.theme.fg2);
      g.drawString(btnText.text, x + bw/2, y + bh/2+2);
    });
    Bangle.setLCDPower(1); // ensure screen is on
  }
  g.reset().clearRect(Bangle.appRect); // clear screen
  if (!message) {
    Bangle.setUI(); // remove watches
    return Promise.resolve();
  }
  draw();
  return new Promise(resolve=>{
    var ui = {mode:"custom", remove: options.remove, redraw: draw, back:options.back, touch:(_,e)=>{
      btnPos.forEach((b,i)=>{
        if (e.x >= b.x1 && e.x <= b.x2 &&
            e.y >= b.y1 && e.y <= b.y2 && !e.hit) {
          e.hit = true; // ensure we don't call twice if the buttons overlap
          draw(i); // highlighted button
          g.flip(); // write to screen
          E.showPrompt(); // remove
          if (e.type===2 /*long press*/ && options.buttonsLong && btns[i] in options.buttonsLong)
            resolve(options.buttonsLong[btns[i]]);
           else
            resolve(options.buttons[btns[i]]);
        }
      });
    }};
    if (btns.length==1 && !options.back) ui.btn = () => {
      draw(0); // highlighted button
      g.flip(); // write to screen
      E.showPrompt(); // remove
      resolve(options.buttons[btns[0]]);
    };
    Bangle.setUI(ui);
  });
})
