/* options = {
  title: text
  buttons : {"Yes":true,"No":false}
} */
(function(msg,options) {
  if (!options) options={};
  if (!options.buttons)
    options.buttons = {"Yes":true,"No":false};
  var loc = require("locale");
  var btns = Object.keys(options.buttons);
  if (!options.selected)
    options.selected = 0;
  function draw() {
    g.reset().setFont("6x8",2).setFontAlign(0,0);
    var W = g.getWidth();
    var H = g.getHeight();
    var title = options.title;
    if (title) {
      title = loc.translate(title);
      g.drawString(title,W/2,34);
      var w = (g.stringWidth(title)+16)/2;
      g.fillRect((W/2)-w,44,(W/2)+w,44);
    }
    var lines = msg.split("\n");
    var offset = (H - lines.length*16)/2;
    lines.forEach((line,y)=>
      g.drawString(loc.translate(line),W/2,offset + y*16));    
    var buttonWidths = 0;
    var buttonPadding = 16;
    btns.forEach(btn=>buttonWidths += buttonPadding+g.stringWidth(loc.translate(btn)));
    var x = (W-buttonWidths)/2;
    var y = H-40;
    btns.forEach((btn,idx)=>{
      btn = loc.translate(btn);
      var w = g.stringWidth(btn);
      x += (buttonPadding+w)/2;      
      var bw = 2+w/2;
      var poly = [x-bw,y-12,
                  x+bw,y-12,
                  x+bw+4,y-8,
                  x+bw+4,y+8,
                  x+bw,y+12,
                  x-bw,y+12,
                  x-bw-4,y+8,
                  x-bw-4,y-8,
                  x-bw,y-12];
      g.setColor(idx==options.selected ? 0x02F7 : 0).fillPoly(poly).setColor(-1).drawPoly(poly).drawString(btn,x,y+1);
      x += (buttonPadding+w)/2;
    });
    g.setColor(-1).flip();  // turn screen on
  }
  
  if (Bangle.btnWatches) {
    Bangle.btnWatches.forEach(clearWatch);
    Bangle.btnWatches = undefined;
  }
  g.clear(1); // clear screen
  Bangle.drawWidgets(); // redraw widgets
  if (!msg) {
    return Promise.resolve();
  }
  draw();
  return new Promise(resolve=>{
    Bangle.btnWatches = [
      setWatch(function() {
        if (options.selected>0) {
          options.selected--;
          draw();
        }
      }, BTN1, {repeat:1}),
      setWatch(function() {
        if (options.selected<btns.length-1) {
          options.selected++;
          draw(); 
        }
      }, BTN3, {repeat:1}),
      setWatch(function() {
        E.showPrompt();
        resolve(options.buttons[btns[options.selected]]);
      }, BTN2, {repeat:1})
    ];
  });
})