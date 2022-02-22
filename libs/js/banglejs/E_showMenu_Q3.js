(function(menu) {
  const H = 40;
  if (menu===undefined) {
    g.clearRect(Bangle.appRect);
    return Bangle.setUI();
  }
  var menuIcon = "\0\f\f\x81\0\xFF\xFF\xFF\0\0\0\0\x0F\xFF\xFF\xF0\0\0\0\0\xFF\xFF\xFF";
  var options = menu[""]||{};
  if (!options.title) options.title="Menu";
  var back = options.back||menu["< Back"];  
  var keys = Object.keys(menu).filter(k=>k!="" && k!="< Back");
  keys.forEach(k => {
    var item = menu[k];
    if ("object" != typeof item) return;
    if ("boolean" == typeof item.value &&
        !item.format)
      item.format = v=>"\0"+atob(v?"EhKBAH//v/////////////5//x//j//H+eP+Mf/A//h//z//////////3//g":"EhKBAH//v//8AA8AA8AA8AA8AA8AA8AA8AA8AA8AA8AA8AA8AA8AA///3//g");
  });
  // Submenu for editing menu options...
  function showSubMenu(item, title) {
    if ("number"!=typeof item.value) 
      return console.log("Unhandled item type");
    var step = item.step||1;
    if (item.min!==undefined && item.max!==undefined &&
        ((item.max-item.min)/step)<20) {
      // show scrolling menu of options
      E.showScroller({
        h : H, c : (item.max+step-item.min)/step,
        scrollMin : -24, scroll : -24, // title is 24px, rendered at -1
        draw : (idx, r) => {
          if (idx<0) // TITLE
            return g.setFont("12x20").setFontAlign(-1,0).drawString(
          menuIcon+" "+title, r.x+12, r.y+H-12);
          g.setColor(g.theme.bg2).fillRect({x:r.x+4,y:r.y+2,w:r.w-8, h:r.h-4, r:5});
          var v = idx*step + item.min;
      g.setColor(g.theme.fg).setFont("12x20").setFontAlign(-1,0).drawString((item.format) ? item.format(v) : v, r.x+12, r.y+H/2);
              g.drawImage(/* 20x20 */atob(v==item.value?"FBSBAAH4AH/gHgeDgBww8MY/xmf+bH/jz/88//PP/zz/88f+Nn/mY/xjDww4AcHgeAf+AB+A":"FBSBAAH4AH/gHgeDgBwwAMYABmAAbAADwAA8AAPAADwAA8AANgAGYABjAAw4AcHgeAf+AB+A"), r.x+r.w-32, r.y+H/2-10);
        },
        select : function(idx) {
          if (idx<0) return; // TITLE
          Bangle.buzz(20);
          item.value = item.min + idx*step;
          if (item.onchange) item.onchange(item.value);
          scr.scroll = s.scroll; // set scroll to prev position
          show(); // redraw original menu
        }
      });
    } else {
      // show simple box for scroll up/down
      var R = Bangle.appRect;
      var v = item.value;
      g.reset().clearRect(Bangle.appRect);
      g.setFont("12x20").setFontAlign(0,0).drawString(
          menuIcon+" "+title, R.x+R.w/2,R.y+12);
      
      function draw() {
        var mx = R.x+R.w/2, my = 12+R.y+R.h/2;
        g.reset().setColor(g.theme.bg2).fillRect({x:R.x+24, y:R.y+36, w:R.w-48, h:R.h-48, r:5});
        g.setColor(g.theme.fg).setFontVector(30).setFontAlign(0,0).drawString(item.format?item.format(v):v, mx, my);
        g.fillPoly([mx,my-45, mx+15,my-30, mx-15,my-30]).fillPoly([mx,my+45, mx+15,my+30, mx-15,my+30]);
      }
      draw();
      Bangle.setUI("updown", dir => {
        if (dir) {
          v -= (dir||1)*(item.step||1);
          if (item.min!==undefined && v<item.min) v = item.wrap ? item.max : item.min;
          if (item.max!==undefined && v>item.max) v = item.wrap ? item.min : item.max;
          draw();
        } else { // actually selected
          item.value = v;
          if (item.onchange) item.onchange(item.value);
          scr.scroll = s.scroll; // set scroll to prev position
          show(); // redraw original menu
        }
      });
    }
  }
  var l = {
    draw : ()=>s.draw()
  };
  
  var scr = {
    h : H, c : keys.length/*title*/,
    scrollMin : -24, scroll : -24, // title is 24px, rendered at -1
    draw : (idx, r) => {
      if (idx<0) // TITLE
        return g.setFont("12x20").setFontAlign(-1,0).drawString(
          menuIcon+" "+options.title, r.x+12, r.y+H-12);
      g.setColor(g.theme.bg2).fillRect({x:r.x+4, y:r.y+2, w:r.w-8, h:r.h-4, r:5});
      g.setColor(g.theme.fg).setFont("12x20");
      var pad = 24;
      var item = menu[keys[idx]];
      if ("object" == typeof item) {
        var v = item.value;
        if (item.format) v=item.format(v);
        if (g.stringMetrics(v).width > r.w/2) // bodge for broken wrapString with image
          v = g.wrapString(v,r.w/2).join("\n");
        g.setFontAlign(1,0).drawString(v,r.x+r.w-8,r.y+H/2);
        pad += g.stringWidth(v);
      } else if ("function" == typeof item) {
        g.drawImage(/* 9x18 */atob("CRKBAGA4Hg8DwPB4HgcDg8PB4eHg8HAwAA=="), r.x+r.w-21, r.y+H/2-9);
        pad += 16;
      }
      var l = g.wrapString(keys[idx],r.w-pad);
      if (l.length>1) g.setFont("6x15");
      g.setFontAlign(-1,0).drawString(l.join("\n"), r.x+12, r.y+H/2);
    },
    select : function(idx) {
      if (idx<0) return back&&back(); // title
      var item = menu[keys[idx]];
      Bangle.buzz(20);
      if ("function" == typeof item) item(l);
      else if ("object" == typeof item) {
        // if a bool, just toggle it
        if ("boolean" == typeof item.value) {
          item.value=!item.value;
          if (item.onchange) item.onchange(item.value);
          s.drawItem(idx);
        } else showSubMenu(item, keys[idx]);
      }
    }
  };
  function show() {
    s = E.showScroller(scr);
    if (back) {
      var touchHandler = (_,e) => {
        if (e.y<24 && e.x<48) back();
      };
      Bangle.on("touch", touchHandler);
      WIDGETS = Object.assign({back:{ area:"tl", width:24, draw:e=>g.reset().setColor("#f00").drawImage(atob("GBiBAAAYAAH/gAf/4A//8B//+D///D///H/P/n+H/n8P/n4f/vwAP/wAP34f/n8P/n+H/n/P/j///D///B//+A//8Af/4AH/gAAYAA=="),e.x,e.y)}},global.WIDGETS);
      Bangle.drawWidgets();
      Bangle.btnWatches = [
        setWatch(function() {
          back();
        }, BTN1, {edge:"falling"}),
      ];
      Bangle.uiRemove = function() {
        Bangle.removeListener("touch", touchHandler);
        delete WIDGETS.back;
        Bangle.drawWidgets();
      };
    }
  }
  show();
  return l;
})