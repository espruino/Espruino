// Typed Array iterating Test

//var a = [0,0,0,0,0,0,0,0];
var a = new Uint8Array(8);
for (i in a) { 
  a[i] = i*2; 
//  print(i+"="+a[i]);
}

var aStr = ""+a;

result = aStr == "0,2,4,6,8,10,12,14";

