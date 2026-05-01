(function(options) {
  /* options = {
    h = height
    c = # of items
    scroll = initial scroll position
    scrollMin = minimum scroll amount (can be negative)
    rect = {x,y,x2,y2,w,h} - area to draw in (defaults to Bangle.appRect)
    draw = function(idx, rect)
    remove = function()
    select = function(idx, touch)
  }

  returns {
    scroll: int                // current scroll amount
    draw: function()           // draw all
    drawItem : function(idx)   // draw specific item
    isActive : function()      // is this scroller still active?
  }

  */
if (!options) return Bangle.setUI(); // remove existing handlers

var drawTimeout, draw = () => {
  if (drawTimeout) {
    clearTimeout(drawTimeout);
    drawTimeout = undefined;
  }
  g.reset().clearRect(R).setClipRect(R.x,R.y,R.x2,R.y2);
  var a = YtoIdx(R.y);
  var b = Math.min(YtoIdx(R.y2),options.c-1);
  for (var i=a;i<=b;i++)
    options.draw(i, {x:R.x,y:idxToY(i),w:R.w,h:options.h}, i==s.selected);
  g.setClipRect(0,0,g.getWidth()-1,g.getHeight()-1);
}, scrollToShow = () => {
  var getScrollAmt = () => { // get amount needed to scroll the item into view
    var y = idxToY(s.selected);
    if (y<80 && (s.scroll>menuScrollMin)) return Math.min(20, 80-y)>>2;
    if (y>160 && (s.scroll<menuScrollMax)) return Math.max(-20, 160-y)>>2;
    return 0;
  };
  if (getScrollAmt()) {
    // auto-scroll if we go off the screen using idxToY? animate?
    if (drawTimeout) clearTimeout(drawTimeout);
    drawTimeout = setTimeout(function scrollOn() {
      drawTimeout=undefined;
      var scroll = getScrollAmt();
      if (!scroll) return;
      ui.drag({dy:scroll});
      setTimeout(scrollOn,20);
    }, 20);
  }
};

var ui = {
  mode : "custom",
  back : options.back,
  remove : options.remove,
  redraw : draw,
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
        if (i<options.c) options.draw(i, {x:R.x,y:y,w:R.w,h:options.h}, i==s.selected);
        i++;
        y += options.h;
      }
    } else { // d>0
      let y = Math.min(R.y+d, R.y2);
      g.setClipRect(R.x,R.y,R.x2,y);
      let i = YtoIdx(y);
      y = idxToY(i);
      while (y > R.y-options.h) {
        if (i<options.c) options.draw(i, {x:R.x,y:y,w:R.w,h:options.h}, i==s.selected);
        y -= options.h;
        i--;
      }
    }
    g.setClipRect(0,0,g.getWidth()-1,g.getHeight()-1);
  }, touch : (_,e)=>{
    if (e.y<R.y-4) return;
    var i = YtoIdx(e.y);
    let yAbs = (e.y + rScroll - R.y);
    let yInElement = yAbs - i*options.h;
    if (e.y>227 && idxToY(i)>227) { // 12px from bottom
      /* If the bottom-most item is only just showing and we
      tap on it, choose the one above instead */
      i--;
      yInElement=options.h-1;
    }
    if ((menuScrollMin<0 || i>=0) && i<options.c){
      //console.log("Press ",e.y,i,yInElement);
      options.select(i, {x:e.x, y:yInElement, type:e.type});
    }
  }, btn : d => {
    Bangle.haptic("btn");
    if (d) { // up/down - change selection
      var old = s.selected;
      if (d<0 && s.selected<0) s.selected = 0;
      s.selected = (s.selected+d+options.c) % options.c;
      if (old>=0) s.drawItem(old);
      s.drawItem(s.selected);
      scrollToShow();
    } else if (s.selected>=0) { // press=select
      options.select(s.selected);
    }
  }
};
Bangle.setUI(ui);

var menuShowing = false;
var R = options.rect||Bangle.appRect;
var Y = R.y;
var n = Math.ceil(R.h/options.h);
var menuScrollMin = 0|options.scrollMin;
var menuScrollMax = options.h*options.c + 60 - R.h; // 60=empty space at bottom
if (menuScrollMax<menuScrollMin) menuScrollMax=menuScrollMin;

function idxToY(i) {
  return i*options.h + R.y - rScroll;
}
function YtoIdx(y) {
  return Math.floor((y + rScroll - R.y)/options.h);
}

var s = {
  scroll : E.clip(0|options.scroll,menuScrollMin,menuScrollMax),
  selected : options.selected ?? -1,
  draw : draw, drawItem : i => {
    var y = idxToY(i);
    g.reset().setClipRect(R.x,Math.max(y,R.y),R.x2,Math.min(y+options.h,R.y2));
    options.draw(i, {x:R.x,y:y,w:R.w,h:options.h}, i==s.selected);
    g.setClipRect(0,0,g.getWidth()-1,g.getHeight()-1);
  }, isActive : () => Bangle.uiRedraw == draw
};
var rScroll = s.scroll&~1; // rendered menu scroll (we only shift by 2 because of dither)
s.draw(); // draw the full scroller
g.flip(); // force an update now to make this snappier
if (s.selected>=0) scrollToShow();
return s;
})