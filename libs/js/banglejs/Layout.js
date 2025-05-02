/* Copyright (c) 2024 Bangle.js contributors. See the file LICENSE for copying permission. */

// See Layout.md for documentation

/* Minify to 'Layout.min.js' by:

   * checking out: https://github.com/espruino/EspruinoDocs
   * run: ../EspruinoDocs/bin/minify.js modules/Layout.js modules/Layout.min.js

*/

function Layout(layout, options) {
  this._l = this.l = layout;
  // Do we have >1 physical buttons?


  this.options = options || {};
  this.lazy = this.options.lazy || false;
  this.physBtns = 1;
  let btnList;
  if (process.env.HWVERSION!=2) {
    this.physBtns = 3;
    // no touchscreen, find any buttons in 'layout'
    btnList = [];
    function btnRecurser(l) {"ram";
      if (l.type=="btn") btnList.push(l);
      if (l.c) l.c.forEach(btnRecurser);
    }
    btnRecurser(layout);
    if (btnList.length) { // there are buttons in 'layout'
      // disable physical buttons - use them for back/next/select
      this.physBtns = 0;
      this.buttons = btnList;
      this.selectedButton = -1;
    }
  }

  if (this.options.btns) {
    var buttons = this.options.btns;
    if (this.physBtns >= buttons.length) {
      // enough physical buttons
      this.b = buttons;
      let btnHeight = Math.floor(Bangle.appRect.h / this.physBtns);
      if (this.physBtns > 2 && buttons.length==1)
        buttons.unshift({label:""}); // pad so if we have a button in the middle
      while (this.physBtns > buttons.length)
        buttons.push({label:""});
      this._l.width = g.getWidth()-8; // text width
      this._l = {type:"h", filly:1, c: [
        this._l,
        {type:"v", pad:1, filly:1, c: buttons.map(b=>(b.type="txt",b.font="6x8",b.height=btnHeight,b.r=1,b))}
      ]};
    } else {
      // add 'soft' buttons
      this._l.width = g.getWidth()-32; // button width
      this._l = {type:"h", c: [
        this._l,
        {type:"v", c: buttons.map(b=>(b.type="btn",b.filly=1,b.width=32,b.r=1,b))}
      ]};
      // if we're selecting with physical buttons, add these to the list
      if (btnList) btnList.push.apply(btnList, this._l.c[1].c);
    }
  }
  // Link in all buttons/touchscreen/etc
  this.setUI();
  // recurse over layout doing some fixing up if needed
  var ll = this;
  function recurser(l) {"ram";
    // add IDs
    if (l.id) ll[l.id] = l;
    // fix type up
    if (!l.type) l.type="";
    if (l.c) l.c.forEach(recurser);
  }
  recurser(this._l);
  this.updateNeeded = true;
}

Layout.prototype.setUI = function() {
  Bangle.setUI(); // remove all existing input handlers

  let uiSet;
  if (this.buttons) {
    // multiple buttons so we'll jus use back/next/select
    Bangle.setUI({mode:"updown", back:this.options.back, remove:this.options.remove}, dir=>{
      var s = this.selectedButton, l=this.buttons.length;
      if (dir===undefined && this.buttons[s])
        return this.buttons[s].cb();
      if (this.buttons[s]) {
        delete this.buttons[s].selected;
        this.render(this.buttons[s]);
      }
      s = (s+l+dir) % l;
      if (this.buttons[s]) {
        this.buttons[s].selected = 1;
        this.render(this.buttons[s]);
      }
      this.selectedButton = s;
    });
    uiSet = true;
  }
  if ((this.options.back || this.options.remove) && !uiSet) Bangle.setUI({mode: "custom", back: this.options.back, remove: this.options.remove});
  // physical buttons -> actual applications
  if (this.b) {
    // Handler for button watch events
    function pressHandler(btn,e) {
      if (e.time-e.lastTime > 0.75 && this.b[btn].cbl)
        this.b[btn].cbl(e);
      else
        if (this.b[btn].cb) this.b[btn].cb(e);
    }
    if (Bangle.btnWatches) Bangle.btnWatches.forEach(clearWatch);
    Bangle.btnWatches = [];
    if (this.b[0]) Bangle.btnWatches.push(setWatch(pressHandler.bind(this,0), BTN1, {repeat:true,edge:-1}));
    if (this.b[1]) Bangle.btnWatches.push(setWatch(pressHandler.bind(this,1), BTN2, {repeat:true,edge:-1}));
    if (this.b[2]) Bangle.btnWatches.push(setWatch(pressHandler.bind(this,2), BTN3, {repeat:true,edge:-1}));
  }
  // Handle touch events on new Bangle.js
  if (process.env.HWVERSION==2) {
    function touchHandler(l,e) {
      if (l.cb && e.x>=l.x && e.y>=l.y && e.x<=l.x+l.w && e.y<=l.y+l.h) {
        if (e.type==2 && l.cbl) l.cbl(e); else if (l.cb) l.cb(e);
      }
      if (l.c) l.c.forEach(n => touchHandler(n,e));
    }
    Bangle.touchHandler = (_,e)=>touchHandler(this._l,e);
    Bangle.on('touch',Bangle.touchHandler);
  }
};

function prepareLazyRender(l, rectsToClear, drawList, rects, parentBg) {
  var bgCol = l.bgCol == null ? parentBg : g.toColor(l.bgCol);
  if (bgCol != parentBg || l.type == "txt" || l.type == "btn" || l.type == "img" || l.type == "custom") {
    // Hash the layoutObject without including its children
    var c = l.c;
    delete l.c;
    var hash = "H"+E.CRC32(E.toJS(l)); // String keys maintain insertion order
    if (c) l.c = c;

    if (!delete rectsToClear[hash]) {
      var r = rects[hash] = [l.x,l.y,l.x+l.w-1,l.y+l.h-1];
      r.bg = parentBg == null ? g.theme.bg : parentBg;
      if (drawList) {
        drawList.push(l);
        drawList = null; // Prevent children from being redundantly added to the drawList
      }
    }
  }

  if (l.c) for (var ch of l.c) prepareLazyRender(ch, rectsToClear, drawList, rects, bgCol);
}

Layout.prototype.render = function (l) {
  if (!l) l = this._l;
  if (this.updateNeeded) this.update();

  var gfx=g; // define locally, because this is faster
  function render(l) {"ram";
    gfx.reset();
    if (l.col!==undefined) gfx.setColor(l.col);
    if (l.bgCol!==undefined) gfx.setBgColor(l.bgCol).clearRect(l.x,l.y,l.x+l.w-1,l.y+l.h-1);
    cb[l.type](l);
  }

  var cb = {
    "":function(){},
    "txt":function(l){"ram";
      if (l.wrap) {
        var lines = gfx.setFont(l.font).setFontAlign(0,-1).wrapString(l.label, l.w);
        var y = l.y+((l.h-gfx.getFontHeight()*lines.length)>>1);
        gfx.drawString(lines.join("\n"), l.x+(l.w>>1), y);
      } else {
        gfx.setFont(l.font).setFontAlign(0,0,l.r).drawString(l.label, l.x+(l.w>>1), l.y+(l.h>>1));
      }
    }, "btn":function(l){"ram";
      var x = l.x+(0|l.pad), y = l.y+(0|l.pad),
          w = l.w-(l.pad<<1), h = l.h-(l.pad<<1);
      var poly = [
        x,y+4,
        x+4,y,
        x+w-5,y,
        x+w-1,y+4,
        x+w-1,y+h-5,
        x+w-5,y+h-1,
        x+4,y+h-1,
        x,y+h-5,
        x,y+4
      ],
      btnborder = l.btnBorderCol!==undefined?l.btnBorderCol:gfx.theme.fg2,
      btnface = l.btnFaceCol!==undefined?l.btnFaceCol:gfx.theme.bg2;
    if(l.selected){
      btnface = gfx.theme.bgH; btnborder = gfx.theme.fgH;
    }
    gfx.setColor(btnface).fillPoly(poly).setColor(btnborder).drawPoly(poly);
    if (l.col!==undefined) gfx.setColor(l.col);
    if (l.src) gfx.setBgColor(btnface).drawImage(
      "function"==typeof l.src?l.src():l.src,
      l.x + l.w/2,
      l.y + l.h/2,
      {scale: l.scale||undefined, rotate: Math.PI*0.5*(l.r||0)}
    );
    else gfx.setFont(l.font||"6x8:2").setFontAlign(0,0,l.r).drawString(l.label,l.x+l.w/2,l.y+l.h/2);
  }, "img":function(l){"ram";
    gfx.drawImage(
      "function"==typeof l.src?l.src():l.src,
      l.x + l.w/2,
      l.y + l.h/2,
      {scale: l.scale||undefined, rotate: Math.PI*0.5*(l.r||0)}
    );
  }, "custom":function(l){ "ram"; l.render(l);
  }, "h":function(l) { "ram"; l.c.forEach(render);
  }, "v":function(l) { "ram"; l.c.forEach(render);
  }};

  if (this.lazy) {
    // we have to use 'var' here not 'let', otherwise the minifier
    // renames vars to the same name, which causes problems as Espruino
    // doesn't yet honour the scoping of 'let'
    if (!this.rects) this.rects = {};
    var rectsToClear = this.rects.clone();
    var drawList = [];
    prepareLazyRender(l, rectsToClear, drawList, this.rects, null);
    for (var h in rectsToClear) delete this.rects[h];
    var clearList = Object.keys(rectsToClear).map(k=>rectsToClear[k]).reverse(); // Rects are cleared in reverse order so that the original bg color is restored
    for (var r of clearList) gfx.setBgColor(r.bg).clearRect.apply(g, r);
    drawList.forEach(render);
  } else { // non-lazy
    render(l);
  }
};

Layout.prototype.forgetLazyState = function () {
  this.rects = {};
};

Layout.prototype.layout = function (l) {
  // l = current layout element
  var floor = Math.floor, cb = {
    "h" : function(l) {"ram";
      var acc_w = l.x + (0|l.pad),
          accfillx = 0,
          fillx = l.c && l.c.reduce((a,l)=>a+(0|l.fillx),0);
      if (!fillx) { acc_w += (l.w-l._w)>>1; fillx=1; }
      var x = acc_w;
      l.c.forEach(c => {
        c.x = 0|x;
        acc_w += c._w;
        accfillx += 0|c.fillx;
        x = acc_w + floor(accfillx*(l.w-l._w)/fillx);
        c.w = 0|(x - c.x);
        c.h = 0|(c.filly ? l.h - (l.pad<<1) : c._h);
        c.y = 0|(l.y + (0|l.pad) + ((1+(0|c.valign))*(l.h-(l.pad<<1)-c.h)>>1));
        if (c.c) cb[c.type](c);
      });
    },
    "v" : function(l) {"ram";
      var acc_h = l.y + (0|l.pad),
          accfilly = 0,
          filly = l.c && l.c.reduce((a,l)=>a+(0|l.filly),0);
      if (!filly) { acc_h += (l.h-l._h)>>1; filly=1; }
      var y = acc_h;
      l.c.forEach(c => {
        c.y = 0|y;
        acc_h += c._h;
        accfilly += 0|c.filly;
        y = acc_h + floor(accfilly*(l.h-l._h)/filly);
        c.h = 0|(y - c.y);
        c.w = 0|(c.fillx ? l.w - (l.pad<<1) : c._w);
        c.x = 0|(l.x + (0|l.pad) + ((1+(0|c.halign))*(l.w-(l.pad<<1)-c.w)>>1));
        if (c.c) cb[c.type](c);
      });
    }
  };
  if (cb[l.type]) cb[l.type](l);
};

Layout.prototype.debug = function(l,c) {
  if (!l) l = this._l;
  c=c||1;
  g.setColor(c&1,c&2,c&4).drawRect(l.x+c-1, l.y+c-1, l.x+l.w-c, l.y+l.h-c);
  if (l.pad)
    g.drawRect(l.x+l.pad-1, l.y+l.pad-1, l.x+l.w-l.pad, l.y+l.h-l.pad);
  c++;
  if (l.c) l.c.forEach(n => this.debug(n,c));
};

Layout.prototype.update = function() {
  delete this.updateNeeded;
  var gfx=g, max=Math.max, rnd=Math.round; // define locally, because this is faster
  // update sizes
  function updateMin(l) {"ram";
    cb[l.type](l);
    if (l.r&1) { // rotation
      var t = l._w;l._w=l._h;l._h=t;
    }
    l._w = max(l._w + (l.pad<<1), 0|l.width);
    l._h = max(l._h + (l.pad<<1), 0|l.height);
  }
  var cb = {
    "txt" : function(l) {"ram";
      if (l.font.endsWith("%"))
        l.font = "Vector"+rnd(gfx.getHeight()*l.font.slice(0,-1)/100);
      if (l.wrap) {
        l._h = l._w = 0;
      } else {
        var m = gfx.setFont(l.font).stringMetrics(l.label);
        l._w = m.width; l._h = m.height;
      }
    }, "btn": function(l) {"ram";
      if (l.font && l.font.endsWith("%"))
        l.font = "Vector"+rnd(gfx.getHeight()*l.font.slice(0,-1)/100);
      var m = l.src?gfx.imageMetrics("function"==typeof l.src?l.src():l.src):gfx.setFont(l.font||"6x8:2").stringMetrics(l.label);
      l._h = 16 + m.height;
      l._w = 20 + m.width;
    }, "img": function(l) {"ram";
      var m = gfx.imageMetrics("function"==typeof l.src?l.src():l.src), s=l.scale||1; // get width and height out of image
      l._w = m.width*s;
      l._h = m.height*s;
    }, "": function(l) {"ram";
      // size should already be set up in width/height
      l._w = 0;
      l._h = 0;
    }, "custom": function(l) {"ram";
      // size should already be set up in width/height
      l._w = 0;
      l._h = 0;
    }, "h": function(l) {"ram";
      l.c.forEach(updateMin);
      l._h = l.c.reduce((a,b)=>max(a,b._h),0);
      l._w = l.c.reduce((a,b)=>a+b._w,0);
      if (l.fillx == null && l.c.some(c=>c.fillx)) l.fillx = 1;
      if (l.filly == null && l.c.some(c=>c.filly)) l.filly = 1;
    }, "v": function(l) {"ram";
      l.c.forEach(updateMin);
      l._h = l.c.reduce((a,b)=>a+b._h,0);
      l._w = l.c.reduce((a,b)=>max(a,b._w),0);
      if (l.fillx == null && l.c.some(c=>c.fillx)) l.fillx = 1;
      if (l.filly == null && l.c.some(c=>c.filly)) l.filly = 1;
    }
  };
  var l = this._l;
  updateMin(l);
  delete cb;
  if (l.fillx || l.filly) { // fill all
    l.w = Bangle.appRect.w;
    l.h = Bangle.appRect.h;
    l.x = Bangle.appRect.x;
    l.y = Bangle.appRect.y;
  } else { // or center
    l.w = l._w;
    l.h = l._h;
    l.x = (Bangle.appRect.w-l.w)>>1;
    l.y = Bangle.appRect.y+((Bangle.appRect.h-l.h)>>1);
  }
  // layout children
  this.layout(l);
};

Layout.prototype.clear = function(l) {
  if (!l) l = this._l;
  g.reset();
  if (l.bgCol!==undefined) g.setBgColor(l.bgCol);
  g.clearRect(l.x,l.y,l.x+l.w-1,l.y+l.h-1);
};

exports = Layout;
