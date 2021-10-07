var g = Graphics.createArrayBuffer(20,20,4);
g.dump = _=>{
  var s = "";
  var n = 0;
  for (var y=0;y<g.getHeight();y++) {
    s+="\n";
    for (var x=0;x<g.getWidth();x++) 
      s+="..\"=+:*#"[g.getPixel(x,y)&7];
  }
  return s;
}
g.print = _=>{
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

// 3 bit 20x20
var img = E.toArrayBuffer(atob("FBSDAAAAAkkkAAAAAAkkkkkAAAAkkkkkkkAAEkkkkkkkgAEkkkkkkkgAkkkkkklkkAkkkkkkv8kEkkkkkl/0kkkkkkkv+kkkklkkl/0kkkkt+kv+kkkkkl/1/0kkkkkkv/+kkkkgkkl/0kkkkAkkkskkkkkAEkkkkkkkgAEkkkkkkkgAAkkkkkkkAAAAkkkkkAAAAAAkkkAAAA=="))

// Normal
g.clear(1).drawImage(img,0,0); 
SHOULD_BE(`
......."""""".......
....."""""""""".....
...""""""""""""""...
..""""""""""""""""..
..""""""""""""""""..
.""""""""""""""*""".
."""""""""""""##*"".
"""""""""""""###""""
""""""""""""###"""""
"""""*"""""###""""""
""""*##"""###"""""""
"""""###"###""""""""
""""""#####"""""""""
.""""""###""""""""".
."""""""*"""""""""".
..""""""""""""""""..
..""""""""""""""""..
...""""""""""""""...
....."""""""""".....
......."""""".......`);

// Force almost identical draw, but with slow path using _jswrap_drawImageLayerGetPixel
g.clear(1).drawImage(img,0,0,{scale:1.0000001}); 
SHOULD_BE(`
......."""""".......
....."""""""""".....
...""""""""""""""...
..""""""""""""""""..
..""""""""""""""""..
.""""""""""""""*""".
."""""""""""""##*"".
"""""""""""""###""""
""""""""""""###"""""
"""""*"""""###""""""
""""*##"""###"""""""
"""""###"###""""""""
""""""#####"""""""""
.""""""###""""""""".
."""""""*"""""""""".
..""""""""""""""""..
..""""""""""""""""..
...""""""""""""""...
....."""""""""".....
......."""""".......`);

result = ok;
