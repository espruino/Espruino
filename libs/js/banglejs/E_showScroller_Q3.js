(function(options) {    
  /* options = {
    h = height
    c = # of items
    scroll = initial scroll position
    scrollMin = minimum scroll amount (can be negative)
    draw = function(idx, rect)
    remove = function()
    select = function(idx, touch)
  }

  returns {
    draw  = draw all
    drawItem(idx) = draw specific item
  }
  */
if (!options) return Bangle.setUI(); // remove existing handlers

var menuShowing = false;
var R = Bangle.appRect;
var Y = Bangle.appRect.y;
var n = Math.ceil(R.h/options.h);
var menuScrollMin = 0|options.scrollMin;
var menuScrollMax = options.h*options.c - R.h;
if (menuScrollMax<menuScrollMin) menuScrollMax=menuScrollMin;

function idxToY(i) {
  return i*options.h + R.y - rScroll;
}
function YtoIdx(y) {
  return Math.floor((y + rScroll - R.y)/options.h);
}
  
var s = {  
  scroll : E.clip(0|options.scroll,menuScrollMin,menuScrollMax),
  draw : () => {
  g.reset().clearRect(R.x,R.y,R.x2,R.y2);
  g.setClipRect(R.x,R.y,R.x2,R.y2);
  var a = YtoIdx(R.y);
  var b = Math.min(YtoIdx(R.y2),options.c-1);
  for (var i=a;i<=b;i++)
    options.draw(i, {x:R.x,y:idxToY(i),w:R.w,h:options.h});
  g.setClipRect(0,0,g.getWidth()-1,g.getHeight()-1);
}, drawItem : i => {
  var y = idxToY(i);
  g.reset().setClipRect(R.x,y,R.x2,y+options.h);
  options.draw(i, {x:R.x,y:y,w:R.w,h:options.h});
  g.setClipRect(0,0,g.getWidth()-1,g.getHeight()-1);
}};
var rScroll = s.scroll&~1; // rendered menu scroll (we only shift by 2 because of dither)
s.draw(); // draw the full scroller
g.flip(); // force an update now to make this snappier
Bangle.setUI({
  mode : "custom",
  back : options.back,
  remove : options.remove,
  drag : e=>{
    var dy = e.dy;
    if (s.scroll - dy > menuScrollMax)
      dy = s.scroll - menuScrollMax;  
    if (s.scroll - dy < menuScrollMin)
      dy = s.scroll - menuScrollMin;
    s.scroll -= dy;
    var oldScroll = rScroll;
    rScroll = s.scroll &~1;
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
  }, touch : (_,e)=>{
    if (e.y<R.y-4) return;
    var i = YtoIdx(e.y);
    if ((menuScrollMin<0 || i>=0) && i<options.c){
      let yAbs = (e.y + rScroll - R.y);
      let yInElement = yAbs - i*options.h;
      options.select(i, {x:e.x, y:yInElement});
    }
  }
});
return s;
})
