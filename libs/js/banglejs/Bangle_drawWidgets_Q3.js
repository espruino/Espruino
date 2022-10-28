(function() {
  if (!global.WIDGETS) return;
  var w=g.getWidth(), h=g.getHeight(), pos={
	  tl:{x:0, y:0, r:0, c:0}, // if r==1, we're right->left
	  tr:{x:w-1, y:0, r:1, c:0},
	  bl:{x:0, y:h-24, r:0, c:0},
	  br:{x:w-1, y:h-24, r:1, c:0}
  }, p;
  for (var wd of WIDGETS) {
    p = pos[wd.area];
  	if (!p) continue;
  	wd.x = p.x - p.r*wd.width;
  	wd.y = p.y;
  	p.x += wd.width*(1-2*p.r);
  	p.c++;
  }
  g.reset();
  if (pos.tl.c || pos.tr.c) g.clearRect(0,0,w-1,23);
  if (pos.bl.c || pos.br.c) g.clearRect(0,h-24,w-1,h-1);
  try { for (wd of WIDGETS) wd.draw(wd); } catch(e) {print(e);}
})
