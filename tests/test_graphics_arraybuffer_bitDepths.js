Graphics.prototype.dump = function(){
  var s = "";
  var n = 0;
  for (var y=0;y<this.getHeight();y++) {
    s+="\n";
    for (var x=0;x<this.getWidth();x++) 
      s+=this.getPixel(x,y)?"#":" ";
  }
  return s;
}
Graphics.prototype.print = function(){
  print("`"+this.dump()+"`");
}

var bits = [1,2,4,8,16,24];
result = 1;
bits.forEach(bpp => {
  console.log("Testing BPP ",bpp);
  var g = Graphics.createArrayBuffer(32,8,bpp,{msb:true});  
  if (E.getAddressOf(g.buffer)==0) throw new Error("Not a flat array! can't test properly");
  var c = g.getColor();
  g.drawLine(0,0,32,32);
  for (var i=0;i<8;i++) g.fillRect(9+i,i, 32-i,i); // test fillRect at different offsets
  //if (bpp<=4) g.print();
  
  if (bpp==1 && btoa(g.buffer)!="gH///0A///8gH//+EA///AgH//gEA//wAgH/4AEA/8A=") { result = 0; print("Data stored wrong"); }
  if (bpp==2 && btoa(g.buffer)!="wAA///////8wAA///////wwAA//////8AwAA//////AAwAA/////wAAwAA////8AAAwAA////AAAAwAA///wAA==") { result = 0; print("Data stored wrong"); }
  if (bpp==4 && btoa(g.buffer)!="8AAAAA///////////////w8AAAAA//////////////8A8AAAAA/////////////wAA8AAAAA////////////AAAA8AAAAA//////////8AAAAA8AAAAA/////////wAAAAAA8AAAAA////////AAAAAAAA8AAAAA//////8AAAA=") { result = 0; print("Data stored wrong"); }
  if (bpp==8 && btoa(g.buffer)!="/wAAAAAAAAAA//////////////////////////////8A/wAAAAAAAAAA/////////////////////////////wAA/wAAAAAAAAAA//////////////////////////8AAAAA/wAAAAAAAAAA////////////////////////AAAAAAAA/wAAAAAAAAAA/////////////////////wAAAAAAAAAA/wAAAAAAAAAA//////////////////8AAAAAAAAAAAAA/wAAAAAAAAAA////////////////AAAAAAAAAAAAAAAA/wAAAAAAAAAA/////////////wAAAAAAAA==") { result = 0; print("Data stored wrong"); }
  if (bpp==16 && btoa(g.buffer)!="//8AAAAAAAAAAAAAAAAAAAAA/////////////////////////////////////////////////////////////wAA//8AAAAAAAAAAAAAAAAAAAAA//////////////////////////////////////////////////////////8AAAAA//8AAAAAAAAAAAAAAAAAAAAA/////////////////////////////////////////////////////wAAAAAAAAAA//8AAAAAAAAAAAAAAAAAAAAA////////////////////////////////////////////////AAAAAAAAAAAAAAAA//8AAAAAAAAAAAAAAAAAAAAA//////////////////////////////////////////8AAAAAAAAAAAAAAAAAAAAA//8AAAAAAAAAAAAAAAAAAAAA/////////////////////////////////////wAAAAAAAAAAAAAAAAAAAAAAAAAA//8AAAAAAAAAAAAAAAAAAAAA////////////////////////////////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA//8AAAAAAAAAAAAAAAAAAAAA//////////////////////////8AAAAAAAAAAAAAAAA=") { result = 0; print("Data stored wrong"); }
  if (bpp==24 && btoa(g.buffer)!="////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA////////////////////////////////////////////////////////////////////////////////////////////AAAA////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA////////////////////////////////////////////////////////////////////////////////////////AAAAAAAA////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA////////////////////////////////////////////////////////////////////////////////AAAAAAAAAAAAAAAA////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA////////////////////////////////////////////////////////////////////////AAAAAAAAAAAAAAAAAAAAAAAA////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA////////////////////////////////////////////////////////////////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA////////////////////////////////////////////////////////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA////////////////////////////////////////////////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA////////////////////////////////////////AAAAAAAAAAAAAAAAAAAAAAAA") { result = 0; print("Data stored wrong"); }
  //print(`  if (bpp==${bpp} && btoa(g.buffer)!=${E.toJS(btoa(g.buffer))}) { result = 0; print("Data stored wrong"); }`);  
  
  
  // test the line reports as being in the right place
  for (var y=0;y<8;y++)
    for (var x=0;x<8;x++) {
      var p = g.getPixel(x,y), exp = ((x==y)?c:0);
      if (p!=exp) {
        print(`Wrong pixel at ${x},${y} (${p} != ${exp})`);
        result = 0;
      }
    }
  
});
