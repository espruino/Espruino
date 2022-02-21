(function(options) {
  /* options = {
    h = height
    count = # of items
    draw = function(idx, rect)
    select = function(idx)
  }*/
  
if (!options) return Bangle.setUI();
var selected = 0;
var w = g.getWidth();
var h = g.getHeight();
var Y = Bangle.appRect.y;
if (Y<24) Y=24; // stop arrow over items if no widgets
var m = w/2;
var n = Math.floor((h-(Y+24))/options.h);

var s = {
  scroll : 0|options.scroll,
  draw : function() {
    g.reset();
    if (selected>=n+s.scroll) s.scroll = 1+selected-n;
    if (selected<s.scroll) s.scroll = selected;
    // draw
    g.setColor(g.theme.fg);
    for (var i=0;i<n;i++) {
      var idx = i+s.scroll;
      if (idx<0 || idx>=options.c) break;
      var y = Y+i*options.h;
      options.draw(idx, {x:0,y:y,w:w,h:options.h});
      // border for selected
      if (i+s.scroll==selected) {
        g.setColor(g.theme.fg).drawRect(0,y,w-1,y+options.h-1).drawRect(1,y+1,w-2,y+options.h-2);
      }
    }
    // arrows
    g.setColor(s.scroll ? g.theme.fg : g.theme.bg);
    g.fillPoly([m,6,m-14,20,m+14,20]);
    g.setColor((options.c>n+s.scroll) ? g.theme.fg : g.theme.bg);
    g.fillPoly([m,h-7,m-14,h-21,m+14,h-21]);
  },
  drawItem : i => {
    var y = Y+(i+s.scroll)*options.h;
    options.draw(i, {x:0,y:y,w:w,h:options.h});
  }
};

g.reset().clearRect(0,Y,w-1,h-1);
s.draw();
Bangle.setUI("updown",dir=>{
  if (dir) {
    selected += dir;
    if (selected<0) selected = options.c-1;
    if (selected>=options.c) selected = 0;
    s.draw();
  } else {
    options.select(selected);
  }
});
return s;
})
