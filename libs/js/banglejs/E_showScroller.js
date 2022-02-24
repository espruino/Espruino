(function(options) {
  /* options = {
    h = height
    c = # of items
    draw = function(idx, rect, selected)
    select = function(idx)
  }*/
  
if (!options) return Bangle.setUI();
var w = Bangle.appRect.w;
var h = Bangle.appRect.h;
var X = Bangle.appRect.x;
var Y = Bangle.appRect.y;

var s = {
  scroll : 0|options.scroll,
  draw : function(idx) {
    g.reset();
    // prefer drawing the list so that the selected item is in the middle of the screen
    var ty=((h-options.h)/2)-s.scroll*options.h;
    var y=ty;
    var by=y+options.c*options.h;
    if (by<=h) y += (h-by);
    if (y>0) y = 0;
    // draw
    for (var i=0;i<options.c;i++) {
      if ((idx===undefined)||(idx===i)) {
        if ((y>-options.h+1)&&(y<h)) {
          var y1 = Math.max(Y,Y+y);
          var y2 = Math.min(Y+h-1,Y+y+options.h-1);
          g.setColor((i==s.scroll)?g.theme.fgH:g.theme.fg)
           .setBgColor((i==s.scroll)?g.theme.bgH:g.theme.bg)
           .setClipRect(X,y1,X+w-1,y2);
          if (!options.draw(i,{x:X,y:Y+y,w:w,h:options.h},i==s.scroll)) {
            // border for selected
            if (i==s.scroll) {
              g.setColor(g.theme.fgH)
               .drawRect(X,Y+y,X+w-1,Y+y+options.h-1)
               .drawRect(X+1,Y+y+1,X+w-2,Y+y+options.h-2);
            }
          }
        }
      }
      y+=options.h;
    }
    // arrows
    g.setClipRect(X,Y,X+w-1,Y+h-1);
    var m=w/2;
    var pt=[X+m,Y,X+m-14,Y+14,X+m+14,Y+14];
    var pb=[X+m,Y+h,X+m-14,Y+h-14,X+m+14,Y+h-14];
    if (ty<0) {
      g.setColor(g.theme.fg)
       .fillPoly(pt)
       .setColor(g.theme.bg)
       .drawPoly(pt,true);
    }
    if (by>h) {
      g.setColor(g.theme.fg)
       .fillPoly(pb)
       .setColor(g.theme.bg)
       .drawPoly(pb,true);
    }
  },
  drawItem : idx => draw(idx)
};

g.reset().clearRect(X,Y,X+w-1,Y+h-1);
s.draw();
Bangle.setUI("updown",dir=>{
  if (dir) {
    s.scroll += dir;
    if (s.scroll<0) s.scroll = options.c-1;
    if (s.scroll>=options.c) s.scroll = 0;
    s.draw();
  } else {
    options.select(s.scroll);
  }
});
return s;
})
