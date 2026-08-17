(function(message,options) {
  if (!options) options={};
  if (!options.buttons)
    options.buttons = {"Yes":true,"No":false};
  var btns = Object.keys(options.buttons), highlightedButton=0;
  if (btns.length>6) throw new Error(">6 buttons");
  var btnPos;
  function draw() {
    g.reset();
    var R = Bangle.appRect, Y = R.y, W = R.w;
    var title = g.findFont(options.title||"", {w:80+Y*2,wrap:1,max:32});// FIXME: use sqrt to work out width?
    if (title.text) {
      Y += title.h+4;
      g.setColor(g.theme.fgH).setBgColor(g.theme.bgH).
        clearRect(0,R.y,239,Y-2).setFontAlign(0,1).
        drawString(title.text, 120, Y);
    } else Y+=4;
    g.setFontAlign(0,0);
    var BX = 0|"0123233"[btns.length],
        BY = Math.ceil(btns.length / BX),
        BH = options.buttonHeight || ((BY>1 || options.img)?40:56),
        TW = [undefined,116,100,50][BX], // allowable text width
        Ws = [undefined,[120],[118,120],[88,58,88]], // button widths
        Xs = [undefined,[60],[0,122],[0,92,154]], // x offsets of bittons
        Ts = [undefined,[120],[80,160],[60,120,180]]; // x offsets of middle of text
    R.y2 -= (BY==1) ? 30 : 15; // bring buttons up a bit
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
      var ix=idx%BX,iy=0|(idx/BX),x = Xs[BX][ix], y = R.y2-(BY-iy)*BH + 2,
          bw = Ws[BX][ix], bh = BH-3;
      btnPos.push({x1:x-2, x2:x+bw-2, y1:y, y2:y+BH});
      var btnText = g.findFont(btn, {w:TW,h:BH-4,wrap:1});
      g.setBgColor(idx===highlightedButton ? g.theme.bgH : g.theme.bg2).clearRect({x:x+1, y:y+1, w:bw-2, h:bh-2, r:11})
       .setColor(idx===highlightedButton ? g.theme.fgH : g.theme.fg2).drawString(btnText.text,Ts[BX][ix],y+2+BH/2);
      if (idx&1) y+=BH;
    });
    Bangle.setLCDPower(1); // ensure screen is on
  }
  if (!message) {
    g.reset().clearRect(Bangle.appRect); // clear screen
    Bangle.setUI(); // remove watches
    return Promise.resolve();
  }
  return new Promise(resolve=>{
    var ui = {mode:"custom", remove: options.remove, redraw: draw, back:options.back, touch:(_,e)=>{
      btnPos.forEach((b,i)=>{
        if (e.x >= b.x1 && e.x <= b.x2 &&
            e.y >= b.y1 && e.y <= b.y2 && !e.hit) {
          e.hit = true; // ensure we don't call twice if the buttons overlap
          highlightedButton = i;
          draw(); // highlighted button
          g.flip(); // write to screen
          Bangle.haptic("touch");
          E.showPrompt(); // remove
          if (e.type===2 /*long press*/ && options.buttonsLong && btns[i] in options.buttonsLong)
            resolve(options.buttonsLong[btns[i]]);
           else
            resolve(options.buttons[btns[i]]);
        }
      });
    }, btn : d => {
      Bangle.haptic("btn");
      if (d) {
        highlightedButton = (highlightedButton+btns.length+d)%btns.length;
        draw(); g.flip(); // write to screen
      } else {
        E.showPrompt(); // remove
        resolve(options.buttons[btns[highlightedButton]]); // FIXME: long press?
      }
    }};
    Bangle.setUI(ui);
    g.reset().clearRect(Bangle.appRect); // clear screen
    draw();
  });
})