// Array.indexOf

var a = [8,9,10];
a["foo"]="lala";
a[3.333] = 3;

var r = [
  a.indexOf(8),
  a.indexOf(10),
  a.indexOf(42)
];

result = r[0]==0 && r[1]==2 && r[2]==-1;

