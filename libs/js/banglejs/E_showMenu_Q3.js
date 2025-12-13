E.showMenu=(function(menu) {
  const H = 40;
  if (menu===undefined) {
    g.clearRect(Bangle.appRect);
    return Bangle.setUI();
  }
  var menuIcon = "\0\f\f\x81\0\xFF\xFF\xFF\0\0\0\0\x0F\xFF\xFF\xF0\0\0\0\0\xFF\xFF\xFF";
  var options = menu[""]||{};
  if (!options.title) options.title="Menu";
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
  function showSubMenu(item, title) {
    /*if ("number"!=typeof item.value)
      return console.log("Unhandled item type");*/
    var step = item.step||1;
    if (!item.noList && item.min!==undefined && item.max!==undefined &&
        ((item.max-item.min)/step)<20) {
      // show scrolling menu of options
      var scroller = E.showScroller({
        h : H, c : (item.max+step-item.min)/step,
        back: show, // redraw original menu
        remove: options.remove,
        scrollMin : -24, scroll : -24, // title is 24px, rendered at -1
        draw : (idx, r) => {
          if (idx<0) {// TITLE
            var titleText = g.findFont(menuIcon+" "+title, {w:g.getWidth()-2,h:24,max:24});
            return g.setFontAlign(-1,0).drawString(titleText.text, r.x+12, r.y+H-12);
          }
          g.setColor(g.theme.bg2).fillRect({x:r.x+4,y:r.y+2,w:r.w-8, h:r.h-4, r:5});
          var v = idx*step + item.min, txt = item.format ? item.format(v,1) : v;
          var itemText = g.findFont(txt, {w:r.w,h:r.h,wrap:1,trim:1});
          g.setColor(g.theme.fg2).setFontAlign(-1,0).drawString(itemText.text, r.x+12, r.y+H/2);
          g.drawImage(/* 20x20 */atob(v==item.value?"FBSBAAH4AH/gHgeDgBww8MY/xmf+bH/jz/88//PP/zz/88f+Nn/mY/xjDww4AcHgeAf+AB+A":"FBSBAAH4AH/gHgeDgBwwAMYABmAAbAADwAA8AAPAADwAA8AANgAGYABjAAw4AcHgeAf+AB+A"), r.x+r.w-32, r.y+H/2-10);
        },
        select : function(idx) {
          if (idx<0) return; // TITLE
          Bangle.buzz(20);
          item.value = item.min + idx*step;
          if (item.onchange) item.onchange(item.value);
          if (scroller.isActive()) { // onchange may have changed menu!
            scr.scroll = l.scroller.scroll; // set scroll to prev position
            show(); // redraw original menu
          }
        }
      });
    } else {
      // show simple box for scroll up/down
      var R = Bangle.appRect;
      var v = item.value;
      g.reset().clearRect(R);
      g.setFont("12x20").setFontAlign(0,0).drawString(
          menuIcon+" "+title, R.x+R.w/2,R.y+12);

      function draw() {
        var mx = R.x+R.w/2, my = 12+R.y+R.h/2, txt = item.format?item.format(v,2):v, s = 30;
        g.reset().setColor(g.theme.bg2).fillRect({x:R.x+24, y:R.y+36, w:R.w-48, h:R.h-48, r:5});
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
            if (dy>0) { dy-=32; cb(1); }
            else { dy+=32; cb(-1); }
            Bangle.buzz(20);
          }
        },
        touch : (_,e) => {
          Bangle.buzz(20);
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
    h : H, c : keys.length/*title*/,
    scrollMin : -32, scroll : options.scroll??-32, // title is 32px high, rendered at -1
    back : back,
    remove : options.remove,
    draw : (idx, r) => {
      g.setFontAlign(-1,0);
      if (idx<0) // TITLE
        return g.drawString(g.findFont(menuIcon+" "+options.title, {w:r.w,h:24,max:24}).text, r.x+12, (r.y+r.h/2)+4);
      var item = menu[keys[idx]], pad = 16;
      var isSeparator = "string" == typeof item && (item == "separator" || item == "Separator")
      if(isSeparator){
        g.setColor(g.theme.fg).drawLine(r.x, r.y+r.h-3, r.w, r.y+r.h-3);
      }else{
        g.setColor(g.theme.bg2).fillRect({x:r.x+4, y:r.y+2, w:r.w-8, h:r.h-4, r:5}).setColor(g.theme.fg2);
      }
      if ("object" == typeof item) {
        var v = item.value;
        if (item.format) v=item.format(v);
        if (v!==undefined) {
          var val = g.findFont(v, {w:r.w/2,h:r.h,wrap:1,trim:1});
          g.setFontAlign(1,0).drawString(val.text,r.x+r.w-8,2+r.y+H/2);
          pad += g.stringWidth(val.text);
          pad+=5;//add more padding so text is not right against the box
        }
      } else if ("function" == typeof item) {
        g.drawImage(/* 9x18 */atob("CRKBAGA4Hg8DwPB4HgcDg8PB4eHg8HAwAA=="), r.x+r.w-21, r.y+H/2-9);
        pad += 16;
      }
      if(isSeparator){
        g.setFontAlign(0).drawString(g.findFont((item&&item.title)??keys[idx], {w:r.w-pad,h:r.h-15,wrap:1,trim:1}).text,r.x+r.w/2, 2+r.y+H/2);
      }else{
        g.setFontAlign(-1,0).drawString(g.findFont((item&&item.title)??keys[idx], {w:r.w-pad,h:r.h,wrap:1,trim:1}).text,r.x+8, 2+r.y+H/2);
      }
    },
    select : function(idx, touch) {
      if (idx<0) return back&&back(); // title
      var item = menu[keys[idx]];
      var isSeparator= "string" == typeof item&&(item=="separator"||item=="Separator")
      if(!isSeparator)Bangle.buzz(20);
      if ("function" == typeof item) item(touch);
      else if ("object" == typeof item) {
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
  function show() {
    l.scroller = E.showScroller(scr);
  }
  show();
  return l;
})
