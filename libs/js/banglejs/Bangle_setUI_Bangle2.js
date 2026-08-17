(function(mode, cb) {
  var options = {},
      hadBackWidget = false;
  if ("object"==typeof mode) {
    options = mode;
    mode = options.mode;
    if (!mode) throw new Error("Missing mode in setUI({...})");
  }
  if (global.WIDGETS && WIDGETS.back) {
    hadBackWidget = true; // if we had a back widget already, don't redraw at the end
    WIDGETS.back.remove(options.back); // only redraw when removing if we don't have options.back
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
  if (Bangle.touchHandler2) {
    Bangle.removeListener("touch", Bangle.touchHandler2);
    delete Bangle.touchHandler2;
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

  if (mode=="updown") {
    if (options.drag) throw new Error("Custom drag handler not supported in mode updown!")
    var dy = 0;
    Bangle.dragHandler = e=>{
      dy += e.dy;
      if (!e.b) dy=0;
      while (Math.abs(dy)>32) {
        Bangle.haptic("drag");
        if (dy>0) { dy-=32; cb(1) }
        else { dy+=32; cb(-1) }
      }
    };
    Bangle.on('drag',Bangle.dragHandler);
    Bangle.touchHandler = d => { Bangle.haptic("touch");cb(); };
    Bangle.on("touch", Bangle.touchHandler);
    Bangle.btnWatches = [
      setWatch(function() { Bangle.haptic("btn");cb(); }, BTN1, {repeat:1, edge:"rising"}),
    ];
  } else if (mode=="leftright") {
    if (options.drag) throw new Error("Custom drag handler not supported in mode leftright!")
    var dx = 0;
    Bangle.dragHandler = e=>{
      dx += e.dx;
      if (!e.b) dx=0;
      while (Math.abs(dx)>32) {
        Bangle.haptic("drag");
        if (dx>0) { dx-=32; cb(1) }
        else { dx+=32; cb(-1) }
      }
    };
    Bangle.on('drag',Bangle.dragHandler);
    Bangle.touchHandler = d => { Bangle.haptic("touch");cb(); };
    Bangle.on("touch", Bangle.touchHandler);
    Bangle.btnWatches = [
      setWatch(function() { Bangle.haptic("btn");cb(); }, BTN1, {repeat:1, edge:"rising"}),
    ];
  } else if (mode=="clock") {
    Bangle.CLOCK=1;
    Bangle.btnWatches = [
      setWatch(function() { Bangle.haptic("btn");Bangle.showLauncher(); }, BTN1, {repeat:1, edge:"rising"}),
    ];
  } else if (mode=="clockupdown") {
    Bangle.CLOCK=1;
    Bangle.touchHandler = (d,e) => {
      if (e.x < 120) return;
      Bangle.haptic("touch");
      cb((e.y > 88) ? 1 : -1);
    };
    Bangle.on("touch", Bangle.touchHandler);
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
  if (options.touch) {
    if (Bangle.touchHandler) // don't overwrite existing touch handler if using updown/etc (#2648)
      Bangle.on("touch", Bangle.touchHandler2 = options.touch);
    else
      Bangle.on("touch", Bangle.touchHandler = options.touch);
  }
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
    // only add back button handler if there's no existing watch on BTN1
    if (Bangle.btnWatches===undefined)
      Bangle.btnWatches = [ setWatch(function() {
        Bangle.btnWatches = undefined; // watch doesn't repeat
        Bangle.haptic("btn");
        options.back();
      }, BTN1, {edge:"rising"}) ];
    // if we have widgets loaded *and* visible at the top, add a back widget (see #3788)
    if (global.WIDGETS && Bangle.appRect.y) {
      // add our own touch handler for touching in the top left
      var touchHandler = function(_,e) {
        if (e.y<36 && e.x<48) {
          e.handled = true;
          E.stopEventPropagation(); // stop subsequent touch handlers from being called
          Bangle.haptic("back");
          options.back();
        }
      };
      Bangle.prependListener("touch", touchHandler);
      // add widget - 'remove' function will remove the widgets
      WIDGETS = Object.assign({back:{ // Object.assign({back..}) ensures this is always the FIRST widget
        area:"tl", width:24,
        draw:e=>g.reset().setColor("#f00").drawImage(atob("GBiBAAAYAAH/gAf/4A//8B//+D///D///H/P/n+H/n8P/n4f/vwAP/wAP34f/n8P/n+H/n/P/j///D///B//+A//8Af/4AH/gAAYAA=="),e.x,e.y),
        remove:function(noclear){
          var w = WIDGETS.back;
          if (w.area!="tl") noclear=true; // area="" is set by widget_utils.hide, so avoid drawing
          Bangle.removeListener("touch", touchHandler);
          if (!noclear) g.reset().clearRect({x:w.x, y:w.y, w:24,h:24});
          delete WIDGETS.back;
          if (!noclear) Bangle.drawWidgets();
        }
      }},global.WIDGETS)
      if (!hadBackWidget) Bangle.drawWidgets();
    }
  }
})
