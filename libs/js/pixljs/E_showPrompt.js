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
    g.setFontAlign(0,0);
    var W = g.getWidth();
    var H = g.getHeight();
    if (options.title) {
      g.drawString(options.title,W/2,4);
      var w = (g.stringWidth(options.title)+16)/2;
      g.fillRect((W/2)-w,8,(W/2)+w,8);
    }
    var lines = msg.split("\n");
    var offset = (H - lines.length*6)/2;
    lines.forEach((line,y)=>
      g.drawString(line,W/2,offset + y*6));    
    var buttonWidths = 0;
    var buttonPadding = 10;
    btns.forEach(btn=>buttonWidths += buttonPadding+g.stringWidth(btn));
    var x = (W-buttonWidths)/2;
    var y = H-7;
    btns.forEach((btn,idx)=>{
      var w = g.stringWidth(btn);
      x += (buttonPadding+w)/2;
      var bw = 2+w/2;
      var poly = [x-bw,y-6,
                  x+bw,y-6,
                  x+bw+2,y-4,
                  x+bw+2,y+4,
                  x+bw,y+6,
                  x-bw,y+6,
                  x-bw-2,y+4,
                  x-bw-2,y-4,
                  x-bw,y-6];
      g.drawPoly(poly);
      if (idx==options.selected) {
        g.setColor(1);
        g.fillPoly(poly);
        g.setColor(0);
      }
      g.drawString(btn,x,y+1);
      g.setColor(1);
      x += (buttonPadding+w)/2;
    });
    g.setColor(1);
    g.drawImage(E.toString(8,8,1,
      0b00010000,
      0b00110000,
      0b01110000,
      0b11111110,
      0b01110000,
      0b00110000,
      0b00010000,
      0b00010000
    ),0,4);
    g.drawImage(E.toString(8,8,1,
      0b00010000,
      0b00011000,
      0b00011100,
      0b00011110,
      0b11111110,
      0b00011100,
      0b00011000,
      0b00010000
    ),0,H-12);
    g.drawImage(E.toString(16,8,1,
      0b01100111,0b10100000,
      0b10010100,0b00100000,
      0b10000100,0b00100000,
      0b01100111,0b00100000,
      0b00010100,0b00100000,
      0b10010100,0b00100000,
      0b01100111,0b10111100,
      0b00000000,0b00000000
    ),W-16,H-8);
    g.flip();
  }
  
  if (Pixl.btnWatches) {
    Pixl.btnWatches.forEach(clearWatch);
    Pixl.btnWatches = undefined;
  }
  if (!msg) {
    g.clear(1);
    g.flip();
    return Promise.resolve();
  }
  draw();
  return new Promise(resolve=>{
    Pixl.btnWatches = [
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
      }, BTN4, {repeat:1}),
      setWatch(function() {
        E.showPrompt();
        resolve(options.buttons[btns[options.selected]]);
      }, BTN3, {repeat:1})
    ];
  });
})