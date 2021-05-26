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
  function b() {
    try{Bangle.buzz(20);}catch(e){}
  }
  if (!mode) return;
  else if (mode=="updown") {
    Bangle.btnWatches = [
      setWatch(function() { b();cb(-1); }, BTN1, {repeat:1}),
      setWatch(function() { b();cb(1); }, BTN3, {repeat:1}),
      setWatch(function() { b();cb(); }, BTN2, {repeat:1})
    ];
  } else if (mode=="leftright") {
    Bangle.btnWatches = [
      setWatch(function() { b();cb(-1); }, BTN1, {repeat:1}),
      setWatch(function() { b();cb(1); }, BTN3, {repeat:1}),
      setWatch(function() { b();cb(); }, BTN2, {repeat:1})
    ];
    Bangle.swipeHandler = d => {b();cb(d);};
    Bangle.on("swipe", Bangle.swipeHandler);
    Bangle.touchHandler = d => {b();cb();};
    Bangle.on("touch", Bangle.touchHandler);
  } else
    throw new Error("Unknown UI mode");
})
