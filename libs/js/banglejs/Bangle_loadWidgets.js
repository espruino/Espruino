(function() {
  global.WIDGETS={};
  require("Storage").list(/\.wid\.js$/).forEach(widget=>{try { eval(require("Storage").read(widget)); } catch (e) {print(widget,e);}}); 
  var W = WIDGETS;
  WIDGETS = {};
  Object.keys(W).sort((a,b)=>(0|W[b].sortorder)-(0|W[a].sortorder)).forEach(k=>WIDGETS[k]=W[k]);
})
