(function(options) {    
  /* options = {
    h = height
    c = # of items
    draw = function(idx, rect)
    select = function(idx)
  }*/
Bangle.setUI(); // remove existing handlers
if (!options) return;
var menuScroll = 0;
var rScroll = 0; // rendered menu scroll (we only shift by 2 because of dither)
var menuShowing = false;
var w = g.getWidth();
var h = g.getHeight();
var Y = Bangle.appRect.y;
var n = Math.ceil((h-Y)/options.h);
var menuScrollMax = options.h*options.c - (h-Y);

function idxToY(i) {
  return i*options.h + Y - rScroll;
}
function YtoIdx(y) {
  return Math.floor((y + rScroll - Y)/options.h);
}

function drawMenu() {
  g.reset().clearRect(0,Y,w-1,h-1);
  g.setClipRect(0,Y,w-1,h-1);
  var a = YtoIdx(Y);
  var b = YtoIdx(h-1);
  for (var i=a;i<=b;i++)
    options.draw(i, {x:0,y:idxToY(i),w:w,h:options.h});
  g.setClipRect(0,0,w-1,h-1);
}
drawMenu();
g.flip(); // force an update now to make this snappier

Bangle.dragHandler = e=>{
  var dy = e.dy;
  if (menuScroll - dy < 0)
    dy = menuScroll;
  if (menuScroll - dy > menuScrollMax)
    dy = menuScroll - menuScrollMax;
  menuScroll -= dy;
  var oldScroll = rScroll;
  rScroll = menuScroll &~1;
  dy = oldScroll-rScroll;
  if (!dy) return;
  g.reset().setClipRect(0,Y,g.getWidth()-1,g.getHeight()-1);
  g.scroll(0,dy);
  var d = e.dy;
  if (d < 0) {
    g.setClipRect(0,h-(1-d),w-1,h-1);
    let i = YtoIdx(h-(1-d));
    let y = idxToY(i);
    while (y < h) {
      options.draw(i, {x:0,y:y,w:w,h:options.h});
      i++;
      y += options.h;
    }
  } else { // d>0
    g.setClipRect(0,Y,w-1,Y+d);
    let i = YtoIdx(Y+d);
    let y = idxToY(i);
    while (y > Y-options.h) {
      options.draw(i, {x:0,y:y,w:w,h:options.h});
      y -= options.h;
      i--;
    }
  }
  g.setClipRect(0,0,w-1,h-1);
};
Bangle.on('drag',Bangle.dragHandler);
Bangle.touchHandler = (_,e)=>{
  if (e.y<20) return;
  var i = YtoIdx(e.y);
  if (i>=0 && i<options.c)
    options.select(i);
};
Bangle.on("touch", Bangle.touchHandler);
})