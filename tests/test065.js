// Null in strings

var a = [
  "\0", 1,
  "A\0", 2,
  "\0A", 2,
  "\0\0\0\0\0\0\0\0A", 9
];

result = 1;
for (i=0;i<a.length;i+=2) 
  if (a[i].length != a[i+1]) result=false;
