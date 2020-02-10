(function(menudata) {
  if (Bangle.btnWatches) {
    Bangle.btnWatches.forEach(clearWatch);
    Bangle.btnWatches = undefined;
  }
  g.clear(1);g.flip(); // clear screen if no menu supplied
  Bangle.drawWidgets();
  if (!menudata) return;
  if (!menudata[""]) menudata[""]={};
  var w = g.getWidth()-9;
  var h = g.getHeight();
  menudata[""].fontHeight=16;
  menudata[""].x=0;
  menudata[""].x2=w-2;
  menudata[""].y=24;
  menudata[""].y2=220;
  menudata[""].cB=0x0007;
  menudata[""].cHlB=0x02F7;
  menudata[""].cHlF=-1;
  menudata[""].predraw=function() {
    g.setFont('6x8',2);g.setFontAlign(-1,-1,0);
  };
  menudata[""].preflip=function(g,less,more) {
    g.drawImage(E.toString(8,8,1,
      0b00010000,
      0b00111000,
      0b01111100,
      0b11111110,
      0b00010000,
      0b00010000,
      0b00010000,
      0b00010000,
    ),w,40);
    g.drawImage(E.toString(8,8,1,
      0b00010000,
      0b00010000,
      0b00010000,
      0b00010000,
      0b11111110,
      0b01111100,
      0b00111000,
      0b00010000,
    ),w,194);
    g.drawImage(E.toString(8,8,1,
      0b00000000,
      0b00001000,
      0b00001100,
      0b00001110,
      0b11111111,
      0b00001110,
      0b00001100,
      0b00001000,
    ),w,116);
    g.setColor(more?-1:0);
    g.fillPoly([104,220,136,220,120,228]);
  };
  var m = require("graphical_menu").list(g, menudata);
  Bangle.btnWatches = [
    setWatch(function() { m.move(-1); }, BTN1, {repeat:1}),
    setWatch(function() { m.move(1); }, BTN3, {repeat:1}),
    setWatch(function() { m.select(); }, BTN2, {repeat:1})
  ];
  return m;
})
