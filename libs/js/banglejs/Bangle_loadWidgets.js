(function() {
  global.WIDGETS={};
  try { require("Storage").list(/\.wid\.js$/).forEach(widget=>eval(require("Storage").read(widget))); } catch (e) {print(e);}
})
