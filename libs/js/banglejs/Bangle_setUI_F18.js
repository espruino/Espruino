(function(mode, cb) {
  var options = {};
  if ("object"==typeof mode) {
    options = mode;
    mode = options.mode;
    if (!mode) throw new Error("Missing mode in setUI({...})");
  }
  var hadBackWidget = false;
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
  if (mode=="updown") {
    Bangle.btnWatches = [
      setWatch(function() { cb(-1); }, BTN1, {repeat:1,edge:"rising"}),
      setWatch(function() { cb(1); }, BTN3, {repeat:1,edge:"rising"}),
      setWatch(function() { cb(); }, BTN2, {repeat:1,edge:"rising"})
    ];
  } else if (mode=="leftright") {
    Bangle.btnWatches = [
      setWatch(function() { cb(-1); }, BTN1, {repeat:1,edge:"rising"}),
      setWatch(function() { cb(1); }, BTN3, {repeat:1,edge:"rising"}),
      setWatch(function() { cb(); }, BTN2, {repeat:1,edge:"rising"})
    ];
    Bangle.swipeHandler = d => {cb(d);};
    Bangle.on("swipe", Bangle.swipeHandler);
    Bangle.touchHandler = d => {cb();};
    Bangle.on("touch", Bangle.touchHandler);
  } else if (mode=="clock") {
    Bangle.CLOCK=1;
    Bangle.btnWatches = [
      setWatch(Bangle.showLauncher, BTN2, {repeat:1,edge:"rising"})
    ];
  } else if (mode=="clockupdown") {
    Bangle.CLOCK=1;
    Bangle.btnWatches = [
      setWatch(function() { cb(-1); }, BTN1, {repeat:1,edge:"rising"}),
      setWatch(function() { cb(1); }, BTN3, {repeat:1,edge:"rising"}),
      setWatch(Bangle.showLauncher, BTN2, {repeat:1,edge:"rising"})
    ];
  } else if (mode=="custom") {
    if (options.clock) {
      Bangle.btnWatches = [
        setWatch(Bangle.showLauncher, BTN2, {repeat:1,edge:"rising"})
      ];
    }
  } else
    throw new Error("Unknown UI mode "+E.toJS(mode));
  if (options.clock) Bangle.CLOCK=1;
  if (options.touch) {
    Bangle.touchHandler = options.touch;
    Bangle.on("touch", Bangle.touchHandler);
  }
  if (options.swipe) {
    Bangle.swipeHandler = options.swipe;
    Bangle.on("swipe", Bangle.swipeHandler);
  }
  if ((options.btn || options.btnRelease) && !Bangle.btnWatches) Bangle.btnWatches = [];
  if (options.btn) Bangle.btnWatches.push(
    setWatch(function() { options.btn(1); }, BTN1, {repeat:1,edge:"rising"}),
    setWatch(function() { options.btn(2); }, BTN2, {repeat:1,edge:"rising"}),
    setWatch(function() { options.btn(3); }, BTN3, {repeat:1,edge:"rising"})
  );
  if (options.btnRelease) Bangle.btnWatches.push(
    setWatch(function() { options.btn(1); }, BTN1, {repeat:1,edge:"falling"}),
    setWatch(function() { options.btn(2); }, BTN2, {repeat:1,edge:"falling"}),
    setWatch(function() { options.btn(3); }, BTN3, {repeat:1,edge:"falling"})
  );
  if (options.remove) // handler for removing the UI (intervals/etc)
    Bangle.uiRemove = options.remove;
  if (options.redraw) // handler for redrawing the UI
    Bangle.uiRedraw = options.redraw;
  if (options.back) {
    var btnWatch;
    // only add back button handler if there's no existing watch on BTN1
    if (Bangle.btnWatches===undefined) 
      btnWatch = setWatch(function() {
        btnWatch = undefined;
        options.back();
      }, BTN3, {edge:"rising"});
    // if we have widgets loaded *and* visible at the top, add a back widget (see #3788)
    if (global.WIDGETS && Bangle.appRect.y) {
      // add our own touch handler for touching in the left
      var touchHandler = function(z) {
        if (z==1) {
          E.stopEventPropagation(); // stop subsequent touch handlers from being called
          options.back();
        }
      };
      Bangle.prependListener("touch", touchHandler);
      // add widget - 'remove' function will remove the widgets
      WIDGETS.back = {
        area:"tl", width:24,
        draw:e=>g.reset().setColor("#f00").drawImage(atob("GBiBAAAYAAH/gAf/4A//8B//+D///D///H/P/n+H/n8P/n4f/vwAP/wAP34f/n8P/n+H/n/P/j///D///B//+A//8Af/4AH/gAAYAA=="),e.x,e.y),
        remove:function(noclear){
          var w = WIDGETS.back;
          if (w.area!="tl") noclear=true; // area="" is set by widget_utils.hide, so avoid drawing
          if (btnWatch) clearWatch(btnWatch);
          Bangle.removeListener("touch", touchHandler);
          if (!noclear) g.reset().clearRect({x:w.x, y:w.y, w:24,h:24});
          delete WIDGETS.back;
          if (!noclear) Bangle.drawWidgets();
        }
      };
      if (!hadBackWidget) Bangle.drawWidgets();
    }
  }
})
