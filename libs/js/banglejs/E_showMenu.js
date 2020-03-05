(function(items) {
  if (Bangle.btnWatches) {
    Bangle.btnWatches.forEach(clearWatch);
    Bangle.btnWatches = undefined;
  }
  g.clear(1);g.flip(); // clear screen if no menu supplied
  Bangle.drawWidgets();
  if (!items) return;
  var w = g.getWidth()-9;
  var h = g.getHeight();
  var menuItems = Object.keys(items);
  var options = items[""];
  if (options) menuItems.splice(menuItems.indexOf(""),1);
  if (!(options instanceof Object)) options = {};
  options.fontHeight=16;
  options.x=0;
  options.x2=w-2;
  options.y=24;
  options.y2=220;
  if (options.selected === undefined)
    options.selected = 0;
  if (!options.fontHeight)
    options.fontHeight = 6;
  var x = 0|options.x;
  var x2 = options.x2||(g.getWidth()-1);
  var y = 0|options.y;
  var y2 = options.y2||(g.getHeight()-1);
  if (options.title)
    y += options.fontHeight+2;
  var cBg = 0x0007; // background col
  var cFg = -1; // foreground col
  var cHighlightBg = 0x02F7;
  var cHighlightFg = -1;
  var loc = require("locale");
  var l = {
    draw : function() {
      g.reset();
      g.setColor(cFg);
      g.setFont('6x8',2).setFontAlign(0,-1,0);
      if (options.title) {
        g.drawString(options.title,(x+x2)/2,y-options.fontHeight-2);
        g.drawLine(x,y-2,x2,y-2);
      }

      var rows = 0|Math.min((y2-y) / options.fontHeight,menuItems.length);
      var idx = E.clip(options.selected-(rows>>1),0,menuItems.length-rows);
      var iy = y;
      var less = idx>0;
      while (rows--) {
        var name = menuItems[idx];
        var item = items[name];
        var hl = (idx==options.selected && !l.selectEdit);
        g.setColor(hl ? cHighlightBg : cBg);
        g.fillRect(x,iy,x2,iy+options.fontHeight-1);
        g.setColor(hl ? cHighlightFg : cFg);
        g.setFontAlign(-1,-1);
        g.drawString(loc.translate(name),x,iy);
        if ("object" == typeof item) {
          var xo = x2;
          var v = item.value;
          if (item.format) v=item.format(v);
          v = loc.translate(""+v);
          if (l.selectEdit && idx==options.selected) {
            xo -= 24 + 1;
            g.setColor(cHighlightBg);
            g.fillRect(xo-(g.stringWidth(v)+4),iy,x2,iy+options.fontHeight-1);
            g.setColor(cHighlightFg);
            g.drawImage("\x0c\x05\x81\x00 \x07\x00\xF9\xF0\x0E\x00@",xo,iy+(options.fontHeight-10)/2,{scale:2});
          }
          g.setFontAlign(1,-1);
          g.drawString(v,xo-2,iy);
        }
        g.setColor(cFg);
        iy += options.fontHeight;
        idx++;
      }
      g.setFontAlign(-1,-1);
      var more = idx<menuItems.length;      
      g.drawImage("\b\b\x01\x108|\xFE\x10\x10\x10\x10"/*E.toString(8,8,1,
        0b00010000,
        0b00111000,
        0b01111100,
        0b11111110,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000
      )*/,w,40);
      g.drawImage("\b\b\x01\x10\x10\x10\x10\xFE|8\x10"/*E.toString(8,8,1,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b11111110,
        0b01111100,
        0b00111000,
        0b00010000
      )*/,w,194);
      g.drawImage("\b\b\x01\x00\b\f\x0E\xFF\x0E\f\b"/*E.toString(8,8,1,
        0b00000000,
        0b00001000,
        0b00001100,
        0b00001110,
        0b11111111,
        0b00001110,
        0b00001100,
        0b00001000
      )*/,w,116);
      g.setColor(more?-1:0);
      g.fillPoly([104,220,136,220,120,228]);
      g.flip();
    },
    select : function(dir) {
      var item = items[menuItems[options.selected]];
      if ("function" == typeof item) item(l);
      else if ("object" == typeof item) {
        // if a number, go into 'edit mode'
        if ("number" == typeof item.value)
          l.selectEdit = l.selectEdit?undefined:item;
        else { // else just toggle bools
          if ("boolean" == typeof item.value) item.value=!item.value;
          if (item.onchange) item.onchange(item.value);
        }
        l.draw();
      }
    },
    move : function(dir) {
      if (l.selectEdit) {
        var item = l.selectEdit;
        item.value -= (dir||1)*(item.step||1);
        if (item.min!==undefined && item.value<item.min) item.value = item.min;
        if (item.max!==undefined && item.value>item.max) item.value = item.max;
        if (item.onchange) item.onchange(item.value);
      } else {
        options.selected = (dir+options.selected)%menuItems.length;
        if (options.selected<0) options.selected += menuItems.length;
      }
      l.draw();
    }
  };
  l.draw();
  Bangle.btnWatches = [
    setWatch(function() { l.move(-1); }, BTN1, {repeat:1}),
    setWatch(function() { l.move(1); }, BTN3, {repeat:1}),
    setWatch(function() { l.select(); }, BTN2, {repeat:1})
  ];
  return l;  
})
