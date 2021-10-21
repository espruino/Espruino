(function(mode, cb) {
  if (Bangle.btnWatches) {
    Bangle.btnWatches.forEach(clearWatch);
    delete Bangle.btnWatches;
  }
  if (Bangle.dragHandler) {
    Bangle.removeListener("drag", Bangle.dragHandler);
    delete Bangle.swipeHandler;
  }
  if (Bangle.touchHandler) {
    Bangle.removeListener("touch", Bangle.touchHandler);
    delete Bangle.touchHandler;
  }
  function b() {
    try{Bangle.buzz(30);}catch(e){}
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
    Bangle.on('drag',Bangle.dragHandler);
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
    Bangle.on('drag',Bangle.dragHandler);
    Bangle.touchHandler = d => {b();cb();};
    Bangle.on("touch", Bangle.touchHandler);
    Bangle.btnWatches = [
      setWatch(function() { b();cb(); }, BTN1, {repeat:1}),
    ];
  } else if (mode=="clock") {
    Bangle.CLOCK=1;
    Bangle.btnWatches = [
      setWatch(Bangle.showLauncher, BTN1, {repeat:1,edge:"falling"})
    ];
  } else if (mode=="clockupdown") {
    Bangle.CLOCK=1;
    Bangle.touchHandler = (d,e) => {
      if (e.x < 120) return;
      b();cb((e.y > 88) ? 1 : -1);
    };
    Bangle.on("touch", Bangle.touchHandler);
    Bangle.btnWatches = [
      setWatch(Bangle.showLauncher, BTN1, {repeat:1,edge:"falling"})
    ];
  } else if (mode=="touch") {
    Bangle.touchHandler = (_,e) => {b();cb(e);};
    Bangle.on("touch", Bangle.touchHandler);
  } else
    throw new Error("Unknown UI mode");
})
