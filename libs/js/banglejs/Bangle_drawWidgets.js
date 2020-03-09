(function() {
  var w=g.getWidth(), h=g.getHeight();
  var pos = {
	  tl:{x:32, y:0, r:0}, // if r==1, we're right->left
	  tr:{x:w-32, y:0, r:1},
	  bl:{x:32, y:h-24, r:0},
	  br:{x:w-32, y:h-24, r:1}
  };
  g.reset().clearRect(0,0,w-1,23).clearRect(0,h-24,w-1,h-1);
  if (global.WIDGETS) for (var wd of WIDGETS) {
	  var p = pos[wd.area];
	  if (!p) return;
	  wd.x = p.x - p.r*wd.width;
	  wd.y = p.y;
	  p.x += wd.width*(1-2*p.r);
	  wd.draw(wd);
  }
})