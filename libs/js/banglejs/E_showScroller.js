(function(options) {
  /* options = {
    h = height
    c = # of items
    draw = function(idx, rect, selected)
    select = function(idx)
  }

  returns {
    scroll: int                // current scroll amount
    draw: function()           // draw all
    drawItem : function(idx)   // draw specific item
  }

  */

if (!options) return Bangle.setUI();

var draw = function(idx) {
  g.reset();
  // prefer drawing the list so that the selected item is in the middle of the screen
  var y=Math.floor((h-options.h)/2)-s.scroll*options.h;
  var ty=y+options.c*options.h;
  if (ty<=h) y += (h-ty);
  if (y>0) y = 0;
  ty=y;
  // draw
  for (var i=0;i<options.c;i++) {
    if ((idx===undefined)||(idx===i)) {
      if ((y>-options.h+1)&&(y<h)) {
        g.setColor((i==s.scroll)?g.theme.fgH:g.theme.fg)
         .setBgColor((i==s.scroll)?g.theme.bgH:g.theme.bg)
         .setClipRect(X,Y+Math.max(0,y),X+w-1,Y+Math.min(h,y+options.h)-1);
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
  var p,m=w/2;
  if (ty<0) {
    p=[X+m,Y,X+m-14,Y+14,X+m+14,Y+14];
    g.setColor(g.theme.fg)
     .fillPoly(p)
     .setColor(g.theme.bg)
     .drawPoly(p,true);
  }
  if (y>h) {
    p=[X+m,Y+h,X+m-14,Y+h-14,X+m+14,Y+h-14];
    g.setColor(g.theme.fg)
     .fillPoly(p)
     .setColor(g.theme.bg)
     .drawPoly(p,true);
  }
};

Bangle.setUI({mode:"updown", back:options.back, remove:options.remove, redraw:draw},dir=>{
  if (dir) {
    s.scroll += dir;
    if (s.scroll<0) s.scroll = options.c-1;
    if (s.scroll>=options.c) s.scroll = 0;
    s.draw();
  } else {
    options.select(s.scroll);
  }
});
var R = Bangle.appRect;
var w = R.w, h = R.h, X = R.x, Y = R.y;

var s = {
  scroll : 0|options.scroll,
  draw : draw,
  drawItem : idx => draw(idx),
  isActive : () => Bangle.uiRedraw == draw
};

g.reset().clearRect(R);
s.draw();
return s;
})
