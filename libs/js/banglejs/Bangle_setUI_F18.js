(function(mode, cb) {
  if (Bangle.btnWatches) {
    Bangle.btnWatches.forEach(clearWatch);
    delete Bangle.btnWatches;
  }
  if (Bangle.swipeHandler) {
    Bangle.removeListener("swipe", Bangle.swipeHandler);
    delete Bangle.swipeHandler;
  }
  if (Bangle.touchandler) {
    Bangle.removeListener("touch", Bangle.touchHandler);
    delete Bangle.touchHandler;
  }
  if (!mode) return;
  else if (mode=="updown") {
    Bangle.btnWatches = [
      setWatch(function() { cb(-1); }, BTN1, {repeat:1}),
      setWatch(function() { cb(1); }, BTN3, {repeat:1}),
      setWatch(function() { cb(); }, BTN2, {repeat:1})
    ];
  } else if (mode=="leftright") {
    Bangle.btnWatches = [
      setWatch(function() { cb(-1); }, BTN1, {repeat:1}),
      setWatch(function() { cb(1); }, BTN3, {repeat:1}),
      setWatch(function() { cb(); }, BTN2, {repeat:1})
    ];
    Bangle.swipeHandler = d => {cb(d);};
    Bangle.on("swipe", Bangle.swipeHandler);
    Bangle.touchHandler = d => {cb();};
    Bangle.on("touch", Bangle.touchHandler);
  } else
    throw new Error("Unknown UI mode");
})
