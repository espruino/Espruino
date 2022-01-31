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
var R = Bangle.appRect;
var Y = Bangle.appRect.y;
var n = Math.ceil(R.h/options.h);
var menuScrollMax = options.h*options.c - R.h;

function idxToY(i) {
  return i*options.h + R.y - rScroll;
}
function YtoIdx(y) {
  return Math.floor((y + rScroll - R.y)/options.h);
}

function drawMenu() {
  g.reset().clearRect(R.x,R.y,R.x2,R.y2);
  g.setClipRect(R.x,R.y,R.x2,R.y2);
  var a = YtoIdx(R.y);
  var b = Math.min(YtoIdx(R.y2),options.c-1);
  for (var i=a;i<=b;i++)
    options.draw(i, {x:R.x,y:idxToY(i),w:R.w,h:options.h});
  g.setClipRect(0,0,g.getWidth()-1,g.getHeight()-1);
}
drawMenu();
g.flip(); // force an update now to make this snappier

Bangle.dragHandler = e=>{
  var dy = e.dy;
  if (menuScroll - dy > menuScrollMax)
    dy = menuScroll - menuScrollMax;  
  if (menuScroll - dy < 0)
    dy = menuScroll;
  menuScroll -= dy;
  var oldScroll = rScroll;
  rScroll = menuScroll &~1;
  dy = oldScroll-rScroll;
  if (!dy) return;
  g.reset().setClipRect(R.x,R.y,R.x2,R.y2);
  g.scroll(0,dy);
  var d = e.dy;
  if (d < 0) {
    g.setClipRect(R.x,R.y2-(1-d),R.x2,R.y2);
    let i = YtoIdx(R.y2-(1-d));
    let y = idxToY(i);
    while (y < R.y2) {
      options.draw(i, {x:R.x,y:y,w:R.w,h:options.h});
      i++;
      y += options.h;
    }
  } else { // d>0
    g.setClipRect(R.x,R.y,R.x2,R.y+d);
    let i = YtoIdx(R.y+d);
    let y = idxToY(i);
    while (y > R.y-options.h) {
      options.draw(i, {x:R.x,y:y,w:R.w,h:options.h});
      y -= options.h;
      i--;
    }
  }
  g.setClipRect(0,0,g.getWidth()-1,g.getHeight()-1);
};
Bangle.on('drag',Bangle.dragHandler);
Bangle.touchHandler = (_,e)=>{
  if (e.y<R.y-4) return;
  var i = YtoIdx(e.y);
  if (i>=0 && i<options.c)
    options.select(i);
};
Bangle.on("touch", Bangle.touchHandler);
})