(function() {
  var l = require("Storage").list(/\.info$/).map(app=>{
    try { return require("Storage").readJSON(app); } catch (e) {}
  }).find(app=>app.type=="launch");
  if (l) load(l.src);
  else E.showMessage("Launcher\nnot found");
})
