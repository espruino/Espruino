/* options = {
  title: text
  buttons : {"Yes":true,"No":false}
} */
(function(msg,options) {
  if (!options) options={};
  if (!options.buttons)
    options.buttons = {"Yes":true,"No":false};
  var btns = Object.keys(options.buttons);
  if (!options.selected)
    options.selected = 0;
  function draw() {
    g.clear(1);
    g.setFont("6x8",2);
    g.setFontAlign(0,0);
    var W = g.getWidth();
    var H = g.getHeight();
    if (options.title) {
      g.drawString(options.title,W/2,24);
      var w = (g.stringWidth(options.title)+16)/2;
      g.fillRect((W/2)-w,34,(W/2)+w,35);
    }
    var lines = msg.split("\n");
    var offset = (H - lines.length*16)/2;
    lines.forEach((line,y)=>
      g.drawString(line,W/2,offset + y*16));    
    var buttonWidths = 0;
    var buttonPadding = 16;
    btns.forEach(btn=>buttonWidths += buttonPadding+g.stringWidth(btn));
    var x = (W-buttonWidths)/2;
    var y = H-40;
    btns.forEach((btn,idx)=>{
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
      if (idx==options.selected) {
        g.setColor(0x02F7);
        g.fillPoly(poly);
        g.setColor(-1);
      }
      g.drawPoly(poly);
      g.drawString(btn,x,y+1);
      x += (buttonPadding+w)/2;
    });
    g.setColor(-1);
    g.flip();
  }
  
  if (Bangle.btnWatches) {
    Bangle.btnWatches.forEach(clearWatch);
    Bangle.btnWatches = undefined;
  }
  if (!msg) {
    g.clear(1);
    g.flip();
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