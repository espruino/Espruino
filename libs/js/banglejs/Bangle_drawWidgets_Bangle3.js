(function(){
  if (!global.WIDGETS) return;
  var w=g.getWidth(), h=g.getHeight(), pos={
    tl:{x:80, y:2, r:0, c:0, w:0}, // if r==1, we're right->left
    tr:{x:160, y:2, r:1, c:0, w:0},
    bl:{x:80, y:h-26, r:0, c:0, w:0},
    br:{x:160, y:h-26, r:1, c:0, w:0}
  }, p, wd;
  for (wd of WIDGETS) {
    p = pos[wd.area];
    if (!p || wd.width == 0) continue;
    p.c++;
    p.w+=wd.width;
  }
  if (pos.tl.w+pos.tr.w>60) { // if too big, work out how much to push down
    var d = (pos.tl.w+pos.tr.w)/2,
        y = 120-Math.sqrt(120*120 - d*d)|0;
    pos.tl.y = pos.tr.y = y;
    pos.tl.x = 120-d;
    pos.tr.x = 120+d;
  }
  if (pos.tr.c && !pos.tl.c) pos.tr.x = (w+p.w)/2; // if only tr, centre it
  for (wd of WIDGETS) {
    p = pos[wd.area];
    if (!p || wd.width == 0) continue;
    wd.x = p.x - p.r*wd.width;
    wd.y = p.y;
    p.x += wd.width*(1-2*p.r);
  }
  g.reset();
  if (pos.bl.c || pos.br.c) g.clearRect(0,pos.bl.y,w-1,h-1);
  if (pos.tl.c || pos.tr.c) g.setBgColor(g.theme.bgH).clearRect(0,0,w-1,pos.tl.y+24);
  try { for (wd of WIDGETS) wd.draw(wd); } catch(e) {print(e);}
})