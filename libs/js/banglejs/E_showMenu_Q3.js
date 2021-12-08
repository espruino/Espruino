(function(items) {
  g.reset().clearRect(Bangle.appRect); // clear if no menu supplied
  Bangle.setLCDPower(1); // ensure screen is on
  if (!items) {
    Bangle.setUI();
    return;
  }
  var menuItems = Object.keys(items);
  var options = items[""];
  if (options) menuItems.splice(menuItems.indexOf(""),1);
  if (!(options instanceof Object)) options = {};
  options.fontHeight = options.fontHeight||21;
  if (options.selected === undefined)
    options.selected = 0;
  var ar = Bangle.appRect;
  var x = ar.x;
  var x2 = ar.x2;
  var y = ar.y;
  var y2 = ar.y2 - 12; // padding at end for arrow
  if (options.title)
    y += 22;
  var loc = require("locale");
  var l = {
    lastIdx : 0,
    draw : function(rowmin,rowmax) {
      var rows = 0|Math.min((y2-y) / options.fontHeight,menuItems.length);
      var idx = E.clip(options.selected-(rows>>1),0,menuItems.length-rows);
      if (idx!=l.lastIdx) rowmin=undefined; // redraw all if we scrolled
      l.lastIdx = idx;      
      var iy = y;
      g.reset().setFontAlign(0,-1,0).setFont('12x20');
      if (options.predraw) options.predraw(g);
      if (rowmin===undefined && options.title)
        g.drawString(options.title,(x+x2)/2,y-21).drawLine(x,y-2,x2,y-2).
          setColor(g.theme.fg).setBgColor(g.theme.bg);
      iy += 12;
      g.setColor((idx>0)?g.theme.fg:g.theme.bg).fillPoly([72,iy,104,iy,88,iy-12]);      
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
        g.drawString(loc.translate(name),x+1,iy+1);
        if ("object" == typeof item) {
          var xo = x2;
          var v = item.value;
          if (item.format) v=item.format(v);
          v = loc.translate(""+v);
          if (l.selectEdit && idx==options.selected) {
            xo -= 24 + 1;
            g.setColor(g.theme.bgH).fillRect(xo-(g.stringWidth(v)+4),iy,x2,iy+options.fontHeight-1);
            g.setColor(g.theme.fgH).drawImage("\x0c\x05\x81\x00 \x07\x00\xF9\xF0\x0E\x00@",xo,iy+(options.fontHeight-10)/2,{scale:2});
          }
          g.setFontAlign(1,-1);
          g.drawString(v,xo-2,iy+1);
        }
        g.setColor(g.theme.fg);
        iy += options.fontHeight;
        idx++;
      }
      g.setFontAlign(-1,-1);      
      g.setColor((idx<menuItems.length)?g.theme.fg:g.theme.bg).fillPoly([72,166,104,166,88,174]);
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
      var item = l.selectEdit
      if (item) {
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