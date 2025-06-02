var a = [];
for (var i=0;i<100;i++) {
  a[i]=E.toFlatString(i+"oh hello world a big flat string -------------------------------------------------------------------------------".substr(0,25+Math.randInt(50)));
}
for (var i=0;i<50;i++) { var n = Math.randInt(100); a[n]=n.toString(); }
var pos = E.getAddressOf(a[99]);
print("\nBefore:");
E.dumpFragmentation();
E.defrag();
print("\nAfter:");
E.dumpFragmentation();
for (var i=0;i<100;i++) {
  if (!a[i].startsWith(i)) throw new Error("Entry "+i+" is wrong");
}
result = E.getAddressOf(a[99]) < pos; // it moved!
