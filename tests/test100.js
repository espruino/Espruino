// ArrayBuffer views

var a = new Uint8Array(10);
for (i=0;i<10;i++) a[i]=i;
var b = new Uint8Array(a.buffer, 5,5); // just using a isn't supposed to work (it seems) - maybe we should make it

print("a = "+a);
print("b = "+b);
result = b[0]==5 && b[4]==9;
