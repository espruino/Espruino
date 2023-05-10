Fonts
=====


This directory contains font maps of various fonts, as well
as a tool to interpret them (`scripts/create_custom_font.js`)

Currently this is only used for testing purposes.

You can dump a font map from Espruino using something like this:

```
var W=4,H=6,FONT="4x6";
//var W=6,H=8,FONT="6x8";
var b = Graphics.createArrayBuffer(W*16,H*16,1,{msb:true});
b.clear(1).setFont(FONT);
for (var y=0;y<16;y++) { 
  for (var x=0;x<16;x++) { 
    if (x || y) b.drawString(String.fromCharCode(x+(y*16)),x*W,y*H)
  }
}
g.drawImage(b.asImage());
b.dump();
```
