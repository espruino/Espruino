(function() {
  var l = require("Storage").list(/\.info$/).map(file => {
    var app = require("Storage").readJSON(file,1);
    if (app && app.type == "launch") return app;
  }).find(x=>x);
  if (l) load(l.src);
  else E.showMessage("Launcher\nnot found");
})
