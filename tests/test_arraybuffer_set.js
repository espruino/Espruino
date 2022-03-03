a = new Uint16Array([1,2,3,4,5,6,7])
print("shift back");
a.set(a.subarray(1));
var ra = a.join(",");

a = new Uint16Array([1,2,3,4,5,6,7])
print("shift forward");
a.set(a.subarray(),1)  
var rb = a.join(","); // this used to fail

result = ra=="2,3,4,5,6,7,7"  && rb=="1,1,2,3,4,5,6";
