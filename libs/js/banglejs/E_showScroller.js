(function(options) {
  /* options = {
    h = height
    c = # of items
    draw = function(idx, rect, selected)
    select = function(idx)
  }*/
  
if (!options) return Bangle.setUI();
var selected = 0;
if (options.scroll) selected=options.scroll;
var w = Bangle.appRect.w;
var h = Bangle.appRect.h;
var X = Bangle.appRect.x;
var Y = Bangle.appRect.y;

var s = {
  scroll : () => selected,
  draw : function(idx) {
    g.reset();
    // prefer drawing the list so that the selected item is in the middle of the screen
    var ty=((h-options.h)/2)-selected*options.h;
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
          g.setColor((i==selected)?g.theme.fgH:g.theme.fg)
           .setBgColor((i==selected)?g.theme.bgH:g.theme.bg)
           .setClipRect(X,y1,X+w-1,y2);
          if (!options.draw(i,{x:X,y:Y+y,w:w,h:options.h},i==selected)) {
            // border for selected
            if (i==selected) {
              g.setColor(g.theme.fgH)
               .drawRect(X,Y+y,w-1,Y+y+options.h-1)
               .drawRect(1,Y+y+1,w-2,Y+y+options.h-2);
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
    selected += dir;
    if (selected<0) selected = options.c-1;
    if (selected>=options.c) selected = 0;
    s.draw();
  } else {
    options.select(selected);
  }
});
return s;
});
