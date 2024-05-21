var g = Graphics.createArrayBuffer(16,16,8);
Graphics.prototype.dump = _=>{
  var s = "";
  var b = new Uint8Array(g.buffer);
  var n = 0;
  for (var y=0;y<g.getHeight();y++) {
    s+="\n|";
    for (var x=0;x<g.getWidth();x++) 
      s+= " .:-=+*#%@"[Math.min(9,b[n++])];
    s+="|";
  }
  return s;
}
Graphics.prototype.print = _=>{
  print("`"+g.dump()+"`");
}
var ok = true;
function SHOULD_BE(a) {
  var b = g.dump();
  if (a!=b) {
    console.log("GOT :"+b+"\nSHOULD BE:"+a+"\n================");
    ok = false;
  }
}

g.clear();
g.setColor(100).setBgColor(0);
g.drawRect(4,4,12,12);
g.filter([
    1, 4, 7, 4, 1,
    4,16,26,16, 4,
    7,26,41,26, 7,
    4,16,26,16, 4,
    1, 4, 7, 4, 1
], { w:5, h:5, div:273 });
g.setColor(0).drawRect(4,4,12,12);
SHOULD_BE(`
|                |
|                |
|   .=+*****+=.  |
|  .%@@@@@@@@@%. |
|  =@         @= |
|  +@ @@@@@@@ @+ |
|  *@ @@***@@ @* |
|  *@ @*   *@ @* |
|  *@ @*   *@ @* |
|  *@ @*   *@ @* |
|  *@ @@***@@ @* |
|  +@ @@@@@@@ @+ |
|  =@         @= |
|  .%@@@@@@@@@%. |
|   .=+*****+=.  |
|                |`);
                
                g.clear();
g.setColor(9).setBgColor(0);
g.drawRect(2,2,13,13);
g.filter([
    1, 4, 7, 4, 1,
    4,16,26,16, 4,
    7,26,41,26, 7,
    4,16,26,16, 4,
    1, 4, 7, 4, 1
], { w:5, h:5, div:50, max:2, filter:"max" });
SHOULD_BE(`
|  ::::::::::::  |
| :::::::::::::: |
|::@@@@@@@@@@@@::|
|::@::::::::::@::|
|::@::::::::::@::|
|::@::      ::@::|
|::@::      ::@::|
|::@::      ::@::|
|::@::      ::@::|
|::@::      ::@::|
|::@::      ::@::|
|::@::::::::::@::|
|::@::::::::::@::|
|::@@@@@@@@@@@@::|
| :::::::::::::: |
|  ::::::::::::  |`);



result = ok;


