(function(menudata) {
  if (Pixl.btnWatches) {
    Pixl.btnWatches.forEach(clearWatch);
    Pixl.btnWatches = undefined;
  }
  g.clear();g.flip(); // clear screen if no menu supplied
  if (!menudata) return;
  function im(b) {
    return {
      width:8,height:b.length,bpp:1,buffer:new Uint8Array(b).buffer
    };
  }
  if (!menudata[""]) menudata[""]={};
  g.setFontBitmap();g.setFontAlign(-1,-1,0);
  var w = g.getWidth()-9;
  var h = g.getHeight();
  menudata[""].x=9;
  menudata[""].x2=w-2;
  menudata[""].preflip=function() {
    g.drawImage(im([
      0b00010000,
      0b00111000,
      0b01111100,
      0b11111110,
      0b00010000,
      0b00010000,
      0b00010000,
      0b00010000,
    ]),0,4);
    g.drawImage(im([
      0b00010000,
      0b00010000,
      0b00010000,
      0b00010000,
      0b11111110,
      0b01111100,
      0b00111000,
      0b00010000,
    ]),0,h-12);
    g.drawImage(im([
      0b00000000,
      0b00001000,
      0b00001100,
      0b00001110,
      0b11111111,
      0b00001110,
      0b00001100,
      0b00001000,
    ]),w+1,h-12);
    //g.drawLine(7,0,7,h);
    //g.drawLine(w,0,w,h);
  };
  var m = require("graphical_menu").list(g, menudata);
  Pixl.btnWatches = [
    setWatch(function() { m.move(-1); }, BTN1, {repeat:1}),
    setWatch(function() { m.move(1); }, BTN4, {repeat:1}),
    setWatch(function() { m.select(); }, BTN3, {repeat:1})
  ];
  return m;
})