(function(mode, cb) {
  var options = {};
  if ("object"==typeof mode) {
    options = mode;
    mode = options.mode;
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
  if (Bangle.uiRemove) {
    Bangle.uiRemove();
    delete Bangle.uiRemove;
  }  
  if (!mode) return;
  else if (mode=="updown") {
    Bangle.btnWatches = [
      setWatch(function() { cb(-1); }, BTN1, {repeat:1}),
      setWatch(function() { cb(1); }, BTN3, {repeat:1}),
      setWatch(function() { cb(); }, BTN2, {repeat:1,edge:"falling"})
    ];
  } else if (mode=="leftright") {
    Bangle.btnWatches = [
      setWatch(function() { cb(-1); }, BTN1, {repeat:1}),
      setWatch(function() { cb(1); }, BTN3, {repeat:1}),
      setWatch(function() { cb(); }, BTN2, {repeat:1,edge:"falling"})
    ];
    Bangle.swipeHandler = d => {cb(d);};
    Bangle.on("swipe", Bangle.swipeHandler);
    Bangle.touchHandler = d => {cb();};
    Bangle.on("touch", Bangle.touchHandler);
  } else if (mode=="clock") {
    Bangle.CLOCK=1;
    Bangle.btnWatches = [
      setWatch(Bangle.showLauncher, BTN2, {repeat:1,edge:"falling"})
    ];
  } else if (mode=="clockupdown") {
    Bangle.CLOCK=1;
    Bangle.btnWatches = [
      setWatch(function() { cb(-1); }, BTN1, {repeat:1}),
      setWatch(function() { cb(1); }, BTN3, {repeat:1}),
      setWatch(Bangle.showLauncher, BTN2, {repeat:1,edge:"falling"})
    ];    
  } else
    throw new Error("Unknown UI mode");
})
