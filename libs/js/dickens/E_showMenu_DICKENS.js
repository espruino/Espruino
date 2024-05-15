(function(items, onCancel) {
  if (!global.Dickens) Dickens={}; // for when called with no boot code
  Bangle.setUI(); // clear Bangle.btnWatches

  g.clear(1);
  // clear screen if no menu supplied
  if (!items) return;

  var cHighlightBg = "#304060";
  var cBorderBg = "#305080";
  g.setColor(cBorderBg);
  g.fillArc(-Math.PI*0.285,Math.PI*0.285,96);
  g.fillArc(Math.PI*(1-0.285),Math.PI*1.285,96);
  g.fillRect(41,62,195,62);
  g.fillRect(41,175,195,175);

  var menuItems = Object.keys(items);
  var options = items[""];
  if (options) menuItems.splice(menuItems.indexOf(""),1);
  if (!(options instanceof Object)) options = {};
  options.fontHeight=16;
  if (options.selected === undefined)
    options.selected = 0;
  var x = 38;
  var x2 = 200;
  var y = 65+6;
  var y2 = 174;
  var cBg = 0; // background col
  var cFg = -1; // foreground col
  var cHighlightFg = -1;
  var l = {
    draw : function() {
      g.reset().setColor(cFg).setFontGrotesk16();
      if (options.title) {
        g.setFontAlign(0,-1,0);
        g.setBgColor(cBorderBg).drawString(options.title,119,42);
      }
      g.setBgColor(0);
      var rows = 0|Math.min((y2-y) / options.fontHeight,menuItems.length);
      var idx = E.clip(options.selected-(rows>>1),0,menuItems.length-rows);
      var iy = y;
      var less = idx>0;
      g.setColor(idx>0?cHighlightBg:cBorderBg).fillPoly([111,36,127,36,119,28]);
      while (rows--) {
        var name = menuItems[idx];
        var item = items[name];
        var hl = (idx==options.selected && !l.selectEdit);
        g.setBgColor(hl ? cHighlightBg : cBg);
        g.setColor(hl ? cHighlightFg : cFg);
        g.clearRect(x,iy-1,x2,iy+options.fontHeight-1);
        g.setFontAlign(-1,-1);
        g.drawString(name,x+2,iy);
        if ("object" == typeof item) {
          var xo = x2;
          var v = item.value;
          if (item.format) v=item.format(v);
          if (("number" == typeof v) && item.precision) v=v.toFixed(item.precision);
          if (l.selectEdit && idx==options.selected) {
            xo -= 24 + 1;
            g.setColor(cHighlightBg);
            g.fillRect(xo-(g.stringWidth(v)+8),iy-1,x2,iy+options.fontHeight-1);
            g.setColor(cHighlightFg);
            g.drawImage("\x0c\x05\x81\x00 \x07\x00\xF9\xF0\x0E\x00@",xo,iy+(options.fontHeight-10)/2,{scale:2});
          }
          g.setFontAlign(1,-1);
          g.drawString(v,xo-2,iy);
        }
        iy += options.fontHeight+1;
        idx++;
      }
      g.setFontAlign(-1,-1);
      g.setColor(idx<menuItems.length?cHighlightBg:cBorderBg).fillPoly([110,191,128,191,119,200]);
      g.flip();
    },
    select : function(dir) {
      var item = items[menuItems[options.selected]];
      if ("function" == typeof item) {
        Bangle.setUI(); // clear Bangle.btnWatches
        item(l);
      }
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
        if (Math.abs(item.value)<1e-10) item.value=0;
        if (item.min!==undefined && item.value<item.min) {
          if (item.wrap) item.value = item.max;
          else item.value = item.min;
        }
        if (item.max!==undefined && item.value>item.max) {
          if (item.wrap) item.value = item.min;
          else item.value = item.max;
        }
        if (item.onchange) item.onchange(item.value);
      } else {
        options.selected = (dir+options.selected)%menuItems.length;
        if (options.selected<0) options.selected += menuItems.length;
      }
      l.draw();
    }
  };
  Dickens.buttonIcons=['select',null,'down','up'];
  if (onCancel) Dickens.buttonIcons[1]='back';
  l.draw();
  Dickens.loadSurround&&Dickens.loadSurround();
  Bangle.btnWatches = [
    setWatch(function() { l.move(-1); }, BTN4, {repeat:1}),
    setWatch(function() { l.move(1); }, BTN3, {repeat:1}),
    setWatch(function() { l.select(); }, BTN1, {repeat:1})
  ];
  if (onCancel) Bangle.btnWatches.push(setWatch(onCancel, BTN2, {repeat:1}))
  return l;
})
