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
    g.reset().setClipRect(R.x,R.y,R.x2,R.y2).scroll(0,dy);
    var d = e.dy;
    if (d < 0) {
      let y = Math.max(R.y2-(1-d), R.y);
      g.setClipRect(R.x,y,R.x2,R.y2);
      let i = YtoIdx(y);
      y = idxToY(i);
      while (y < R.y2) {
        options.draw(i, {x:R.x,y:y,w:R.w,h:options.h});
        i++;
        y += options.h;
      }
    } else { // d>0
      let y = Math.min(R.y+d, R.y2);
      g.setClipRect(R.x,R.y,R.x2,y);
      let i = YtoIdx(y);
      y = idxToY(i);
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
  
var menuShowing = false;
var R = Bangle.appRect;
var Y = R.y;
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
    g.reset().clearRect(R).setClipRect(R.x,R.y,R.x2,R.y2);
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
  }
};
var rScroll = s.scroll&~1; // rendered menu scroll (we only shift by 2 because of dither)
s.draw(); // draw the full scroller
g.flip(); // force an update now to make this snappier
return s;
})