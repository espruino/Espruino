(function() {
  var l = require("Storage").list().filter(a=>a[0]=='+').map(app=>{
    try { return require("Storage").readJSON(app); } catch (e) {}
  }).find(app=>app.type=="launch");
  if (l) load(l.src);
  else E.showMessage("Launcher\nnot found");
})