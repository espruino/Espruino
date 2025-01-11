#!bin/espruino

eval(require("fs").readFile("sdl.js"));

R = Bangle.appRect;

function introScreen() {
  g.reset().clearRect(R);
  g.setColor(0,0,0).setFont("Vector",25);
  g.setFontAlign(0,0);
  g.drawString("Benchmark", 85,35);
  g.setColor(0,0,0).setFont("Vector",18);
  g.drawString("Press button", 85,55);
}
function lineBench() {
  /* 500 lines a second on hardware, 125 lines with flip */
  for (let i=0; i<1000; i++) {
    let x1 = Math.random() * 160;
    let y1 = Math.random() * 160;
    let x2 = Math.random() * 160;
    let y2 = Math.random() * 160;
    
    g.drawLine(x1, y1, x2, y2);
    //g.flip();
  }
}
function polyBench() {
  /* 275 hollow polygons a second on hardware, 99 with flip */
  /* 261 filled polygons a second on hardware, 99 with flip */
  for (let i=0; i<1000; i++) {
    let x1 = Math.random() * 160;
    let y1 = Math.random() * 160;
    let x2 = Math.random() * 160;
    let y2 = Math.random() * 160;
    let c = Math.random();
    
    g.setColor(c, c, c);
    g.fillPoly([80, x1, y1, 80, 80, x2, y2, 80], true);
    //g.flip();
  }
}
function checksum(d) {
  let sum = 0;
  for (i=0; i<d.length; i++) {
    sum += (d[i]*1);
  }
  return sum;
}
function linearRead() {
  /* 10000b block -> 8.3MB/sec, 781..877 IOPS
      1000b block -> 920K/sec, 909 IOPS, 0.55 sec
       100b block -> 100K/sec
        10b block -> 10K/sec, 1020 IOPS, 914 IOPS with ops counting
        
      1000b block backwards -- 0.59 sec.
       100b block -- 5.93.
                  backwards -- 6.27
                  random -- 7.13
     checksum 5.97 -> 351 seconds with checksum. 1400bytes/second
   */
     
  let size = 500000;
  let block = 100;
  let i = 0;
  let ops = 0;
  let sum = 0;
  while (i < size) {
    //let pos = Math.random() * size;
    let pos = i;
    //let pos = size-i;
    let d = require("Storage").read("delme.mtar", pos, block);
    //sum += checksum(E.toUint8Array(d));
    i += block;
    ops ++;
  }
  print(ops, "ops", sum);
}
function drawBench(name) {
  g.setColor(0,0,0).setFont("Vector",25);
  g.setFontAlign(0,0);
  g.drawString(name, 85,35);
  g.setColor(0,0,0).setFont("Vector",18);
  g.drawString("Running", 85,55);
  g.flip();
}
function runBench(b, name) {
  drawBench(name);
  g.reset().clearRect(R);

  let t1 = getTime();
  print("--------------------------------------------------");
  print("Running",name);
  b();
  let m = (getTime()-t1) + " sec";
  print("..done in", m);
  drawBench(name);
  g.setColor(0,0,0).setFont("Vector",18);
  g.drawString(m, 85,85);

}
function redraw() {
  //runBench(lineBench, "Lines");
  runBench(polyBench, "Polygons");
  //runBench(linearRead, "Linear read");
}
function showMap() {
  g.reset().clearRect(R);
  redraw();
  emptyMap();
}
function emptyMap() {
  Bangle.setUI({mode:"custom",drag:e=>{
      g.reset().clearRect(R);
      redraw();    
  }, btn: btn=>{
    print("Button pressed");
  }});
}

const st = require('Storage');
const hs = require('heatshrink');

introScreen();
emptyMap();




