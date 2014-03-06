var a = new Uint8Array([1,2,10,20]);

var res = [
  a.interpolate2d(2,0,0), 1,
  a.interpolate2d(2,1,0), 2,
  a.interpolate2d(2,0,1), 10,
  a.interpolate2d(2,1,1), 20,
  a.interpolate2d(2,0.5,0), 1.5,
  a.interpolate2d(2,0.2,0), 1.2,
  a.interpolate2d(2,0.5,1), 15,
  a.interpolate2d(2,0.2,1), 12,
  a.interpolate2d(2,0,0.5), 5.5,
  a.interpolate2d(2,1,0.5), 11 ];

result = 1;

for (var i=0;i<res.length;i+=2)
  if (res[i] < res[i+1]-0.001 || res[i] > res[i+1]+0.001)
    result = 0;

var a = new Uint8Array(8*8);
for (y=0;y<8;y++) for (x=0;x<8;x++) a[x+y*8] = x;
for (var i=0;i<7;i+=0.1) {
  var b =  a.interpolate2d(8,i,0);  
  if (b < i-0.001 || b > i+0.001) {
    print(i + "!="+b);
    result = 0;
  }
}
for (var i=0;i<7;i+=0.1) {
  var b =  a.interpolate2d(8,i,0.5);  
  if (b < i-0.001 || b > i+0.001) {
    print(i + "!="+b);
    result = 0;
  }
}
for (var i=0;i<7;i+=0.1) {
  var b =  a.interpolate2d(8,i,1);  
  if (b < i-0.001 || b > i+0.001) {
    print(i + "!="+b);
    result = 0;
  }
}

for (y=0;y<8;y++) for (x=0;x<8;x++) a[x+y*8] = y;
for (var i=0;i<7;i+=0.1) {
  var b =  a.interpolate2d(8,0,i);  
  if (b < i-0.001 || b > i+0.001) {
    print(i + "!="+b);
    result = 0;
  }
}
for (var i=0;i<7;i+=0.1) {
  var b =  a.interpolate2d(8,0.5,i);  
  if (b < i-0.001 || b > i+0.001) {
    print(i + "!="+b);
    result = 0;
  }
}
for (var i=0;i<7;i+=0.1) {
  var b =  a.interpolate2d(8,1,i);  
  if (b < i-0.001 || b > i+0.001) {
    print(i + "!="+b);
    result = 0;
  }
}
