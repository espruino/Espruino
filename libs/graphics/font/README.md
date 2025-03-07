Fonts
=====


This directory contains font maps of various fonts, as well
as tools to interpret them

## Tools

These are now (mainly) stored in https://github.com/espruino/EspruinoWebTools

## 4x6 and 6x8

These are fixed-width fonts, created from:

* font4x6.png
* font6x8.png

```
node ~/workspace/EspruinoWebTools/cli/fontconverter.js font4x6.png --height 6 --debug --oh bitmap_font_4x6.h
node ~/workspace/EspruinoWebTools/cli/fontconverter.js font6x8.png --height 8 --debug --oh bitmap_font_6x8.h
```

This doesn't generate the exact file, as the code to decode it (`graphicsDrawChar4x6`/etc) is usually on the end.

## Vector Fonts

The vector font is stored in `fontmap_13x19.svg` and is decoded with `build_vector_font.js`

## renaissance Fonts

These fonts are used by Bangle.js 2. They're based on the fonts from https://github.com/pebble-dev/renaissance
but have had the line height reduced (ASCII chars render the same, but some characters with diacritics have
had to be compressed vertically).

These are turned into Espruino files (embedded `pbf` fonts) with the following commands:

```
node ~/workspace/EspruinoWebTools/cli/fontconverter.js renaissance_crop_28.pbff --range All --spaceWidth 6 --opbfc 28 --test "Hello World"
node ~/workspace/EspruinoWebTools/cli/fontconverter.js renaissance_crop_22.pbff --range All --spaceWidth 4 --opbfc 22 --test "Hello World"
node ~/workspace/EspruinoWebTools/cli/fontconverter.js renaissance_crop_17.pbff --range All --spaceWidth 3 --opbfc 17 --test "Hello World"
node ~/workspace/EspruinoWebTools/cli/fontconverter.js renaissance_crop_14.pbff --range All --spaceWidth 3 --opbfc 14 --test "Hello World"
mv jswrap_font_* ..
```

`renaissance_crop_*` were originally created with:

```
node ~/workspace/EspruinoWebTools/cli/fontconverter.js renaissance_28.pbff --range All --height 22 --shiftUp 10 --nudge --debug --spaceWidth 4 --opbff renaissance_crop_22.pbff
node ~/workspace/EspruinoWebTools/cli/fontconverter.js renaissance_24_bold.pbff --range All --height 17 --shiftUp 10 --nudge --debug --spaceWidth 3 --opbff renaissance_crop_17.pbff
node ~/workspace/EspruinoWebTools/cli/fontconverter.js renaissance_18_bold.pbff --range All --height 14 --shiftUp 7 --nudge --debug --spaceWidth 3 --opbff renaissance_crop_14.pbff
node ~/workspace/EspruinoWebTools/cli/fontconverter.js renaissance_18.pbff --range All --height 15 --shiftUp 7 --nudge --doubleSize --opbff renaissance_crop_28.pbff
```

And then patched up by hand. Some glyphs are still not complete so PRs hugely appreciated!

## Testing

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

You can also use EspruinoWebTools to write out text in the font. For example to check positioning of some of the characters:

```
node ~/workspace/EspruinoWebTools/cli/fontconverter.js renaissance_18_doubled.pbff --range All --test "AÁÉÍÓÚÝÀÈÌÒÙÂÊÎÔÛÄËÏÖÜŸŶÃÑÕOZŹŻŽĆĈĊĎ"
node ~/workspace/EspruinoWebTools/cli/fontconverter.js renaissance_18_doubled.pbff --range All --test "aáéíóúýàèìòùâêîôûãñõäëïöüÿzźżžćĉċ"
```