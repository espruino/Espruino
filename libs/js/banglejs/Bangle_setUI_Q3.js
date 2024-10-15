(function(mode, cb) {
  var options = {};
  if ("object"==typeof mode) {
    options = mode;
    mode = options.mode;
    if (!mode) throw new Error("Missing mode in setUI({...})");
  }
  var redraw = true;
  if (global.WIDGETS && WIDGETS.back) {
    redraw = false;
    WIDGETS.back.remove(mode && options.back);
  }
  if (Bangle.btnWatches) {
    Bangle.btnWatches.forEach(clearWatch);
    delete Bangle.btnWatches;
  }
  if (Bangle.swipeHandler) {
    Bangle.removeListener("swipe", Bangle.swipeHandler);
    delete Bangle.swipeHandler;
  }
  if (Bangle.dragHandler) {
    Bangle.removeListener("drag", Bangle.dragHandler);
    delete Bangle.dragHandler;
  }
  if (Bangle.touchHandler) {
    Bangle.removeListener("touch", Bangle.touchHandler);
    delete Bangle.touchHandler;
  }
  delete Bangle.uiRedraw;
  delete Bangle.CLOCK;
  if (Bangle.uiRemove) {
    let r = Bangle.uiRemove;
    delete Bangle.uiRemove; // stop recursion if setUI is called inside uiRemove
    r();
  }
  g.reset();// reset graphics state, just in case
  if (!mode) return;
  function b() {
    try{Bangle.buzz(30);}catch(e){}
  }
  if (mode=="updown") {
    if (options.drag) throw new Error("Custom drag handler not supported in mode updown!")
    var dy = 0;
    Bangle.dragHandler = e=>{
      dy += e.dy;
      if (!e.b) dy=0;
      while (Math.abs(dy)>32) {
        if (dy>0) { dy-=32; cb(1) }
        else { dy+=32; cb(-1) }
        Bangle.buzz(20);
      }
    };
    Bangle.on('drag',Bangle.dragHandler);
    Bangle.touchHandler = d => {b();cb();};
    Bangle.btnWatches = [
      setWatch(function() { b();cb(); }, BTN1, {repeat:1, edge:"rising"}),
    ];
  } else if (mode=="leftright") {
    if (options.drag) throw new Error("Custom drag handler not supported in mode leftright!")
    var dx = 0;
    Bangle.dragHandler = e=>{
      dx += e.dx;
      if (!e.b) dx=0;
      while (Math.abs(dx)>32) {
        if (dx>0) { dx-=32; cb(1) }
        else { dx+=32; cb(-1) }
        Bangle.buzz(20);
      }
    };
    Bangle.on('drag',Bangle.dragHandler);
    Bangle.touchHandler = d => {b();cb();};
    Bangle.btnWatches = [
      setWatch(function() { b();cb(); }, BTN1, {repeat:1, edge:"rising"}),
    ];
  } else if (mode=="clock") {
    Bangle.CLOCK=1;
    Bangle.btnWatches = [
      setWatch(Bangle.showLauncher, BTN1, {repeat:1,edge:"rising"})
    ];
  } else if (mode=="clockupdown") {
    Bangle.CLOCK=1;
    Bangle.touchHandler = (d,e) => {
      if (e.x < 120) return;
      b();cb((e.y > 88) ? 1 : -1);
    };
    Bangle.btnWatches = [
      setWatch(Bangle.showLauncher, BTN1, {repeat:1,edge:"rising"})
    ];
  } else if (mode=="custom") {
    if (options.clock) {
      Bangle.btnWatches = [
        setWatch(Bangle.showLauncher, BTN1, {repeat:1,edge:"rising"})
      ];
    }
  } else
    throw new Error("Unknown UI mode "+E.toJS(mode));
  if (options.clock) Bangle.CLOCK=1;
  if (options.touch)
  Bangle.touchHandler = options.touch;
  if (options.drag) {
    Bangle.dragHandler = options.drag;
    Bangle.on("drag", Bangle.dragHandler);
  }
  if (options.swipe) {
    Bangle.swipeHandler = options.swipe;
    Bangle.on("swipe", Bangle.swipeHandler);
  }
  if ((options.btn || options.btnRelease) && !Bangle.btnWatches) Bangle.btnWatches = [];
  if (options.btn) Bangle.btnWatches.push(setWatch(options.btn.bind(options), BTN1, {repeat:1,edge:"rising"}))
  if (options.btnRelease) Bangle.btnWatches.push(setWatch(options.btnRelease.bind(options), BTN1, {repeat:1,edge:"falling"}))
  if (options.remove) // handler for removing the UI (intervals/etc)
    Bangle.uiRemove = options.remove;
  if (options.redraw) // handler for redrawing the UI
    Bangle.uiRedraw = options.redraw;
  if (options.back) {
    var touchHandler = (_,e) => {
      if (e.y<36 && e.x<48) {
        e.handled = true;
        E.stopEventPropagation();
        options.back();
      }
    };
    Bangle.on("touch", touchHandler);
    // If a touch handler was needed for setUI, add it - but ignore touches if they've already gone to the 'back' handler
    if (Bangle.touchHandler) {
      var uiTouchHandler = Bangle.touchHandler;
      Bangle.touchHandler = (_,e) => {
        if (!e.handled) uiTouchHandler(_,e);
      };
      Bangle.on("touch", Bangle.touchHandler);
    }
    var btnWatch;
    if (Bangle.btnWatches===undefined) // only add back button handler if there's no existing watch on BTN1
    btnWatch = setWatch(function() {
      btnWatch = undefined;
      options.back();
    }, BTN1, {edge:"rising"});
    WIDGETS = Object.assign({back:{
      area:"tl", width:24,
      draw:e=>g.reset().setColor("#f00").drawImage(atob("GBiBAAAYAAH/gAf/4A//8B//+D///D///H/P/n+H/n8P/n4f/vwAP/wAP34f/n8P/n+H/n/P/j///D///B//+A//8Af/4AH/gAAYAA=="),e.x,e.y),
      remove:(noclear)=>{
        if (btnWatch) clearWatch(btnWatch);
        Bangle.removeListener("touch", touchHandler);
        if (!noclear) g.reset().clearRect({x:WIDGETS.back.x, y:WIDGETS.back.y, w:24,h:24});
        delete WIDGETS.back;
        if (!noclear) Bangle.drawWidgets();
      }
    }},global.WIDGETS);
    if (redraw) Bangle.drawWidgets();
  } else { // If a touch handler was needed for setUI, add it
    if (Bangle.touchHandler)
      Bangle.on("touch", Bangle.touchHandler);
  }
})
