(function(options) {
  /* options = {
    h = height
    count = # of items
    draw = function(idx, rect)
    select = function(idx)
  }*/
  
if (!options) return Bangle.setUI();
var selected = 0;
var menuScroll = 0|option.scroll;
var menuShowing = false;
var w = g.getWidth();
var h = g.getHeight();
var Y = Bangle.appRect.y;
var m = w/2;
var n = Math.floor((h-(Y+24))/options.h);

function drawMenu() {
  g.reset();
  if (selected>=n+menuScroll) menuScroll = 1+selected-n;
  if (selected<menuScroll) menuScroll = selected;
  // draw
  g.setColor(g.theme.fg);
  for (var i=0;i<n;i++) {
    var idx = i+menuScroll;
    if (idx<0 || idx>=options.c) break;
    var y = Y+i*options.h;
    options.draw(idx, {x:0,y:y,w:w,h:options.h});
    // border for selected
    if (i+menuScroll==selected) {
      g.setColor(g.theme.fg).drawRect(0,y,w-1,y+options.h-1).drawRect(1,y+1,w-2,y+options.h-2);
    }
  }
  // arrows
  g.setColor(menuScroll ? g.theme.fg : g.theme.bg);
  g.fillPoly([m,6,m-14,20,m+14,20]);
  g.setColor((options.c>n+menuScroll) ? g.theme.fg : g.theme.bg);
  g.fillPoly([m,h-7,m-14,h-21,m+14,h-21]);
}
g.reset().clearRect(0,Y,w-1,h-1);
drawMenu();
Bangle.setUI("updown",dir=>{
  if (dir) {
    selected += dir;
    if (selected<0) selected = options.c-1;
    if (selected>=options.c) selected = 0;
    drawMenu();
  } else {
    options.select(selected);
  }
});
return {
  draw : drawMenu,
  drawItem : i => {
    var y = Y+(i+menuScroll)*options.h;
    options.draw(i, {x:0,y:y,w:w,h:options.h});
  }
};
})
