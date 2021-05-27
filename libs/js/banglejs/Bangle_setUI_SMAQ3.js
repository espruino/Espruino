(function(mode, cb) {
  if (Bangle.btnWatches) {
    Bangle.btnWatches.forEach(clearWatch);
    delete Bangle.btnWatches;
  }
  if (Bangle.dragHandler) {
    E.removeListener("touch", Bangle.dragHandler);
    delete Bangle.swipeHandler;
  }
  if (Bangle.touchHandler) {
    Bangle.removeListener("touch", Bangle.touchHandler);
    delete Bangle.touchHandler;
  }
  function b() {
    try{Bangle.buzz(20);}catch(e){}
  }
  if (!mode) return;
  else if (mode=="updown") {
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
    E.on('touch',Bangle.dragHandler);
    Bangle.touchHandler = d => {b();cb();};
    Bangle.on("touch", Bangle.touchHandler);
    Bangle.btnWatches = [
      setWatch(function() { b();cb(); }, BTN1, {repeat:1}),
    ];
  } else if (mode=="leftright") {
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
    E.on('touch',Bangle.dragHandler);
    Bangle.touchHandler = d => {b();cb();};
    Bangle.on("touch", Bangle.touchHandler);
    Bangle.btnWatches = [
      setWatch(function() { b();cb(); }, BTN1, {repeat:1}),
    ];
  } else
    throw new Error("Unknown UI mode");
})
