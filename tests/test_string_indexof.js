var a = "ABCD";

var r = [
a.indexOf("A"), 0,
a.indexOf("ABCD"), 0,
a.indexOf("BC"), 1,
a.indexOf("C"), 2,
a.indexOf("D"), 3,
a.indexOf("CD"), 2,
a.indexOf("X"), -1,
a.indexOf("ABCDEFG"), -1,
a.indexOf("QVWXYZ"), -1,
a.indexOf("VWXYZ"), -1,
a.indexOf("WXYZ"), -1,
];

result = 1;
for (i=0;i<r.length;i+=2)
  if (r[i]!=r[i+1])
    result=0;


