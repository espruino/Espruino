// currently has wrong number of elements (iterator issue?)

var a = new Uint8Array(2);
a[1] = 1;
result = JSON.stringify(new Uint8Array(1))=="new Uint8Array(1)" && JSON.stringify(a)=="new Uint8Array([0,1])";
