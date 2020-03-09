(function() {
  var w=g.getWidth(), h=g.getHeight();
  var pos = {
	  tl:{x:28, y:0, r:0}, // if r==1, we're right->left
	  tr:{x:w-28, y:0, r:1},
	  bl:{x:24, y:h-24, r:0},
	  br:{x:w-24, y:h-24, r:1}
  };
  if (global.WIDGETS) {
    for (var wd of WIDGETS) {
	  var p = pos[wd.area];
	  if (!p) return;
	  wd.x = p.x - p.r*wd.width;
	  wd.y = p.y;
	  p.x += wd.width*(1-2*p.r);
	  wd.draw(wd);
    }
    g.reset().clearRect(0,0,pos.tl.x,23).clearRect(0,h-24,pos.bl.x,h-1); // left
    g.clearRect(pos.tr.x,0,w-1,23).clearRect(pos.br.x,h-24,w-1,h-1); // right
    for (wd of WIDGETS) wd.draw(wd);
  }
})