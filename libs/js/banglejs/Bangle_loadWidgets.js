(function() {
  global.WIDGETS={};
  require("Storage").list(/\.wid\.js$/).forEach(widget=>eval(require("Storage").read(widget)));
})
