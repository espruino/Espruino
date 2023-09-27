(function(mode, cb) {
  if (Bangle.btnWatches) {
    Bangle.btnWatches.forEach(w => {
      try { clearWatch(w); } catch (e) {}
    });
    delete Bangle.btnWatches;
  }
  // this is just a replacement for E.clearWatches for the moment
})
