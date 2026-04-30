(function(menu){
  const H = 40;
  var R = Bangle.appRect, top = R.y;
  if (menu===undefined) {
    g.clearRect(R);
    return Bangle.setUI();
  }
  var options = menu[""]||{};
  if (!options.title) options.title="Menu";
  var title = g.findFont(options.title, {w:80+top*2,wrap:0,max:32,min:12}); // FIXME: use sqrt to work out width?
  R.y += title.h+4; R.h -= title.h+4; // push down for title
  var back = options.back||menu["< Back"];
  var keys = Object.keys(menu).filter(k=>k!=="" && k!="< Back");
  keys.forEach(k => {
    var item = menu[k];
    if ("object" != typeof item) return;
    if ("boolean" == typeof item.value &&
        !item.format)
      item.format = v=>"\0"+atob(v?"EhKBAH//v/////////////5//x//j//H+eP+Mf/A//h//z//////////3//g":"EhKBAH//v//8AA8AA8AA8AA8AA8AA8AA8AA8AA8AA8AA8AA8AA8AA///3//g");
  });
  // Submenu for editing menu options...
  function showSubMenu(item, titleText) {
    var R = Bangle.appRect;
    var title = g.findFont(titleText, {w:80+top,wrap:0,max:32,min:12}); // FIXME: use sqrt to work out width?
    R.y += title.h+4; R.h -= title.h+4; // push down for title
    /*if ("number"!=typeof item.value)
      return console.log("Unhandled item type");*/
    // title
    showTitle(R,title);
    var step = item.step||1;
    if (!item.noList && item.min!==undefined && item.max!==undefined &&
        ((item.max-item.min)/step)<20) {
      // show scrolling menu of options
      var scroller = E.showScroller({
        h : H, c : 1+(item.max+step-item.min)/step,
        // FIXME: set selected to closest to original value
        back: show, // redraw original menu
        remove: options.remove,
        rect : { x:0, y:R.y, x2:R.x2, y2:R.y2, w:R.w, h:R.h },
        draw : (idx, r, selected) => {
          var v = idx*step + item.min, txt = item.format ? item.format(v,1) : v;
          if (v>item.max) return;
          g.setBgColor(selected?g.theme.bgH:g.theme.bg2).clearRect({x:r.x+16, y:r.y+2, w:r.w-32, h:r.h-4, r:5});
          var itemText = g.findFont(txt, {w:r.w,h:r.h,wrap:1,trim:1});
          g.setColor(g.theme.fg2).setFontAlign(-1,0).drawString(itemText.text, r.x+24, r.y+H/2);
          g.drawImage(/* 20x20 */atob(v==item.value?"FBSBAAH4AH/gHgeDgBww8MY/xmf+bH/jz/88//PP/zz/88f+Nn/mY/xjDww4AcHgeAf+AB+A":"FBSBAAH4AH/gHgeDgBwwAMYABmAAbAADwAA8AAPAADwAA8AANgAGYABjAAw4AcHgeAf+AB+A"), r.x+r.w-44, r.y+H/2-10);
        },
        select : function(idx) {
          var v = idx*step + item.min;
          if (v>item.max) return;
          Bangle.haptic("touch");
          item.value = v;
          if (item.onchange) item.onchange(item.value);
          if (scroller.isActive()) { // onchange may have changed menu!
            scr.scroll = l.scroller.scroll; // set scroll to prev position
            show(); // redraw original menu
          }
        }
      });
    } else {
      // show simple box for scroll up/down
      var v = item.value;
      function draw() {
        var mx = R.x+R.w/2, my = R.y+R.h/2 - 12, txt = item.format?item.format(v,2):v, s = 30;
        g.reset().clearRect({x:R.x+24, y:R.y+36, w:R.w-48, h:R.h-48, r:5});
        g.setColor(g.theme.fg2).setFontVector(Math.min(30,(R.w-52)*100/g.setFontVector(100).stringWidth(txt))).setFontAlign(0,0).drawString(txt, mx, my);
        g.fillPoly([mx,my-45, mx+15,my-30, mx-15,my-30]).fillPoly([mx,my+45, mx+15,my+30, mx-15,my+30]);
      }
      function cb(dir) {
        if (dir) {
          v -= (dir||1)*(item.step||1);
          if (item.min!==undefined && v<item.min) v = item.wrap ? item.max : item.min;
          if (item.max!==undefined && v>item.max) v = item.wrap ? item.min : item.max;
          draw();
        } else { // actually selected
          item.value = v;
          if (item.onchange) item.onchange(item.value);
          if (Bangle.uiRedraw == draw) { // onchange may have changed menu!
            scr.scroll = l.scroller.scroll; // set scroll to prev position
            show(); // redraw original menu
          }
        }
      }
      draw();
      var dy = 0;
      Bangle.setUI({
        mode: "custom",
        back: show,
        remove: options.remove,
        redraw : draw,
        drag : e => {
          dy += e.dy; // after a certain amount of dragging up/down fire cb
          if (!e.b) dy=0;
          while (Math.abs(dy)>32) {
            Bangle.haptic("drag");
            if (dy>0) { dy-=32; cb(1); }
            else { dy+=32; cb(-1); }
          }
        },
        touch : (_,e) => {
          Bangle.haptic("touch");
          if (e.y<82) cb(-1); // top third
          else if (e.y>142) cb(1); // bottom third
          else cb(); // middle = accept
        }
      });
    }
  }
  var l = {
    draw : ()=>l.scroller.draw(),
    scroller : undefined
  };
  var scr = {
    h : H, c : keys.length+1/*title*/,
    rect : { x:0, y:R.y, x2:R.x2, y2:R.y2, w:R.w, h:R.h },
    back : back,
    remove : options.remove,
    draw : (idx, r, selected) => {
      if (idx>=keys.length) return;
      g.setFontAlign(-1,0).setBgColor(selected?g.theme.bgH:g.theme.bg2).clearRect({x:r.x+16, y:r.y+2, w:r.w-32, h:r.h-4, r:5}).setColor(g.theme.fg2);
      var item = menu[keys[idx]], pad = 40;
      if ("object" == typeof item) {
        var v = item.value;
        if (item.format) v=item.format(v);
        if (v!==undefined) {
          var val = g.findFont(v, {w:r.w/2,h:r.h,wrap:1,trim:1});
          g.setFontAlign(1,0).drawString(val.text,r.x+r.w-20,2+r.y+H/2);
          pad += g.stringWidth(val.text);
        }
      } else if ("function" == typeof item) {
        g.drawImage(/* 9x18 */atob("CRKBAGA4Hg8DwPB4HgcDg8PB4eHg8HAwAA=="), r.x+r.w-33, r.y+H/2-9);
        pad += 16;
      }
      g.setFontAlign(-1,0).drawString(g.findFont((item&&item.title)??keys[idx], {w:r.w-pad,h:r.h,wrap:1,trim:1}).text, r.x+20, 2+r.y+H/2);
    },
    select : function(idx, touch) {
      var item = menu[keys[idx]];
      if ("function" == typeof item) {
        Bangle.haptic("touch");
        item(touch);
      } else if ("object" == typeof item) {
        Bangle.haptic("touch");
        if ("number" == typeof item.value) {
          showSubMenu(item, keys[idx]);
        } else {
          // if a bool, just toggle it
          if ("boolean"==typeof item.value)
            item.value=!item.value;
          if (item.onchange) item.onchange(item.value, touch);
          if (l.scroller.isActive()) l.scroller.drawItem(idx);
        }
      }
    }
  };
  function showTitle(R,title) {
    g.reset().clearRect(R).setBgColor(g.theme.bgH).clearRect(20,top,220,R.y-2).setFontAlign(0,1).setFont(title.font).drawString(title.text, 120, R.y-2);
  }
  function show() {
    showTitle(R,title);
    l.scroller = E.showScroller(scr);
  }
  show();
  return l;
})