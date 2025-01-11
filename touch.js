eval(require("fs").readFile("sdl.js"));

function touchHandler(d) {
  let x = Math.floor(d.x);
  let y = Math.floor(d.y);

  if (1) { /* Just a debugging feature */
    g.setColor(0.25, 0, 0);
    g.fillCircle(x, y, 5);
  }
}

function touchTest() {
  g.setColor(0,0,0).setFont("Vector",25);
  g.setFontAlign(0,0);
  g.drawString("SDL test", 85,35);
  g.setColor(0,0,1).setFont("Vector",18);
  g.drawString("input", 85,55);

  Bangle.on("drag", touchHandler);
}

touchTest();
