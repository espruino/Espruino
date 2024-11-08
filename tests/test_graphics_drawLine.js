var g = Graphics.createArrayBuffer(16,16,8);
g.dump = _=>{
  var s = "";
  var b = new Uint8Array(g.buffer);
  var n = 0;
  for (var y=0;y<g.getHeight();y++) {
    s+="\n";
    for (var x=0;x<g.getWidth();x++) 
      s+= " .:-=+*#%@"[b[n++]];
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

g.clear();
g.setColor(9).setBgColor(0);
g.drawLine(1,1,14,8);
g.drawLineAA(1,1+4,14,8+4);

g.setColor(1);
g.drawLine(10,0,10,0);
g.drawLine(10,1,11,1);
g.drawLine(10,2,12,2);
g.drawLine(10,3,13,3);

SHOULD_BE(`
          .     
 @        ..    
  @@      ...   
    @@    ....  
      @@        
 @=     @@      
  +%=     @@    
   .+%-     @@  
     .*#:     @ 
       :#*:     
         -#*.   
           -%+  
             =@ 
                
                
                `);

result = ok;


