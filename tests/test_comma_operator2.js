// From http://www.espruino.com/SSD1306
// ... fails after 1v88 with ES6 support
var f=new Uint8Array([174,213,128,168,63,211,0,64,141,20,32,0,161,200,218,18,129,207,217,241,219,64,164,166,175]),h=[33,0,127,34,0,7];
function k(b){b&&(b.height&&(f[4]=b.height-1,f[15]=64==b.height?18:2,h[5]=(b.height>>3)-1),void 0!==b.contrast&&(f[17]=b.contrast))};
k({ height:32});

result = f[4]==31;
