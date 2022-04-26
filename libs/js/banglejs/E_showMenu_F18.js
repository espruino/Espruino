(function(items) {
  g.reset().clearRect(Bangle.appRect); // clear if no menu supplied
  Bangle.setLCDPower(1); // ensure screen is on
  if (!items) {
    Bangle.setUI();
    return;
  }
  var options = items[""];  
  var menuItems = Object.keys(items);
  if (options) {
    menuItems.splice(menuItems.indexOf(""),1);
    if (options.back) { // handle 'options.back'
      items["< Back"] = options.back;
      menuItems.unshift("< Back");
    }
  }
  // auto-fill boolean values with no format
  menuItems.forEach(k => {
    var item = items[k];
    if ("object" != typeof item) return;
    if ("boolean" == typeof item.value && !item.format)
      item.format = v=>atob(v?"AAwMggC///7//////////8///w///D/y8P/4A//8D/////////+///4=":"AAwMgQD/+AGAGAGAGAGAGAGAGAGAH/8=");
  });  
  if (!(options instanceof Object)) options = {};
  options.fontHeight = options.fontHeight||16;
  if (options.selected === undefined)
    options.selected = 0;
  if (!options.fontHeight)
    options.fontHeight = 6;
  var ar = Bangle.appRect;
  var x = ar.x;
  var x2 = ar.x2-11; // padding at side for up/down
  var y = ar.y;
  var y2 = ar.y2 - 20; // padding at end for arrow
  if (options.title)
    y += options.fontHeight+2;
  var l = {
    lastIdx : 0,
    draw : function(rowmin,rowmax) {
      var rows = 0|Math.min((y2-y) / options.fontHeight,menuItems.length);
      var idx = E.clip(options.selected-(rows>>1),0,menuItems.length-rows);
      if (idx!=l.lastIdx) rowmin=undefined; // redraw all if we scrolled
      l.lastIdx = idx;      
      var more = (idx+rows)<menuItems.length;
      var iy = y;
      g.reset().setFont('6x8',2).setFontAlign(0,-1,0);
      if (options.predraw) options.predraw(g);
      if (rowmin===undefined && options.title) {
        g.drawString(options.title,(x+x2)/2,y-options.fontHeight-2);
        g.drawLine(x,y-2,x2,y-2);
      }
      if (rowmin!==undefined) {
        if (idx<rowmin) {
          iy += options.fontHeight*(rowmin-idx);
          idx=rowmin;
        }
        if (idx+rows>rowmax) {
          rows = 1+rowmax-rowmin;
        }
      }
      while (rows--) {
        var name = menuItems[idx];
        var item = items[name];
        var hl = (idx==options.selected && !l.selectEdit);
        g.setColor(hl ? g.theme.bgH : g.theme.bg);
        g.fillRect(x,iy,x2,iy+options.fontHeight-1);
        g.setColor(hl ? g.theme.fgH : g.theme.fg);
        g.setFontAlign(-1,-1);
        g.drawString(name,x,iy);
        if ("object" == typeof item) {
          var xo = x2;
          var v = item.value;
          if (item.format) v=item.format(v);
          if (l.selectEdit && idx==options.selected) {
            xo -= 24 + 1;
            g.setColor(g.theme.bgH).fillRect(xo-(g.stringWidth(v)+4),iy,x2,iy+options.fontHeight-1);
            g.setColor(g.theme.fgH).drawImage("\x0c\x05\x81\x00 \x07\x00\xF9\xF0\x0E\x00@",xo,iy+(options.fontHeight-10)/2,{scale:2});
          }
          g.setFontAlign(1,-1);
          g.drawString(v,xo-2,iy);
        }
        g.setColor(g.theme.fg);
        iy += options.fontHeight;
        idx++;
      }
      g.setFontAlign(-1,-1);
      g.drawImage("\b\b\x01\x108|\xFE\x10\x10\x10\x10"/*E.toString(8,8,1,
        0b00010000,
        0b00111000,
        0b01111100,
        0b11111110,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000
      )*/,x2+2,40);
      g.drawImage("\b\b\x01\x10\x10\x10\x10\xFE|8\x10"/*E.toString(8,8,1,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b11111110,
        0b01111100,
        0b00111000,
        0b00010000
      )*/,x2+2,194);
      g.drawImage("\b\b\x01\x00\b\f\x0E\xFF\x0E\f\b"/*E.toString(8,8,1,
        0b00000000,
        0b00001000,
        0b00001100,
        0b00001110,
        0b11111111,
        0b00001110,
        0b00001100,
        0b00001000
      )*/,x2+2,116);
      g.setColor(more?g.theme.fg:g.theme.bg).fillPoly([104,220,136,220,120,228]);
      g.flip();
    },
    select : function() {
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
      var item = l.selectEdit;
      if (item) {
        item = l.selectEdit;
        item.value -= (dir||1)*(item.step||1);
        if (item.min!==undefined && item.value<item.min) item.value = item.wrap ? item.max : item.min;
        if (item.max!==undefined && item.value>item.max) item.value = item.wrap ? item.min : item.max;
        if (item.onchange) item.onchange(item.value);
        l.draw(options.selected,options.selected);
      } else {
        var lastSelected=options.selected;
        options.selected = (dir+options.selected+menuItems.length)%menuItems.length;
        l.draw(Math.min(lastSelected,options.selected), Math.max(lastSelected,options.selected));
      }
    }
  };
  l.draw();
  Bangle.setUI("updown",dir => {
    if (dir) l.move(dir);
    else l.select();
  });
  return l;
})
