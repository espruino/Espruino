(function(msg,options) {
  if (!global.Dickens) Dickens={}; // for when called with no boot code
  var cHighlightBg = "#304060";
  var cBorderBg = "#305080";
  if (!options) options={};
  if (!options.buttons)
    options.buttons = {"Yes":true,"No":false};
  var btns = Object.keys(options.buttons);
  if (!options.selected)
    options.selected = 0;
  if (options.vstack===undefined)
    options.vstack = 1;
  function draw() {
    g.reset();
    g.setColor(cBorderBg);
    g.fillArc(-Math.PI*0.285,Math.PI*0.285,96);
    g.fillArc(Math.PI*(1-0.285),Math.PI*1.285,96);
    g.fillRect(41,62,195,62);
    g.fillRect(41,175,195,175);
    g.setColor(-1);
    var W = g.getWidth();
    var H = g.getHeight();
    var title = options.title;
    if (title) {
      g.setFontGrotesk16().setFontAlign(0,-1,0).setBgColor(cBorderBg).drawString(title,119,42).setBgColor(0);
    }
    var i =options.icon;
    if (i) {
      g.setBgColor(cBorderBg).drawImage(i.img, i.x, i.y);
    }
    g.setFontGrotesk16().setFontAlign(0,0,0);
    var lines = msg.split("\n");
    var offset = 105 - lines.length*16/2;
    lines.forEach((line,y)=>
      g.drawString(line,W/2,offset + y*16));
    var buttonWidths = 0;
    var buttonPadding = 16;
    var x, y, w, bw, poly;
    if (options.vstack) {
      x = 120;
      y = 172-btns.length*18;
      btns.forEach((btn,idx)=>{
        bw = 50;
        poly = [x-bw-4,y-9,x+bw+4,y-9,x+bw+4,y+9,x-bw-4,y+9];
        g.setColor(idx==options.selected ? cHighlightBg : 0).fillPoly(poly).setColor(-1).drawPoly(poly,1).setFontGrotesk14().drawString(btn,x,y+1);
        y += 18;
      });
    } else
    {
      btns.forEach(btn=>buttonWidths += buttonPadding+g.stringWidth(btn));
      x = (W-buttonWidths)/2;
      y = 150;
      btns.forEach((btn,idx)=>{
        w = g.stringWidth(btn);
        x += (buttonPadding+w)/2;
        bw = 2+w/2;
        poly = [x-bw-4,y-10,x+bw+4,y-10,x+bw+4,y+10,x-bw-4,y+10];
        g.setColor(idx==options.selected ? cHighlightBg : 0).fillPoly(poly).setColor(-1).drawPoly(poly,1).drawString(btn,x,y+1);
        x += (buttonPadding+w)/2;
      });
    }
    g.setColor(-1).flip();  // turn screen on
  }
  Bangle.setUI(); // clear Bangle.btnWatches
  // TODO: Widgets?
  g.clear(1);
  Dickens.buttonIcons=['select',null,'down','up'];
  if (!msg) {
    return Promise.resolve();
  }
  draw();
  Dickens.loadSurround&&Dickens.loadSurround();
  return new Promise(resolve=>{
    Bangle.btnWatches = [
      setWatch(function() {
        if (options.selected>0) {
          options.selected--;
          draw();
        }
      }, BTN4, {repeat:1}),
      setWatch(function() {
        if (options.selected<btns.length-1) {
          options.selected++;
          draw();
        }
      }, BTN3, {repeat:1}),
      setWatch(function() {
        E.showPrompt();
        Bangle.setUI(); // clear Bangle.btnWatches
        resolve(options.buttons[btns[options.selected]]);
      }, BTN1, {repeat:1})
    ];
  });
})