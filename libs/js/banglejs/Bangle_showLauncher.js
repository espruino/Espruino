(function() {
  var l = require("Storage").list(/\.info$/).map(file => {
    var app = require("Storage").readJSON(file,1);
    if (app && app.type == "launch") return app;
  }).find(x=>x);
  if (l) {
    if (Bangle.uiRemove) {
      Bangle.setUI(); // remove all existing UI (and call Bangle.uiRemove)
      setTimeout(eval,0,require("Storage").read(l.src)); // Load launcher direct without a reboot
    } else load(l.src);
  } else E.showMessage("Launcher\nnot found");
})
