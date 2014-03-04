// test continue

var c1=0;
var c3=0;

for (i=0;i<10;i++) {
  if (i>4 && i<8) continue;
  c1++;
}

for (j=0;j<10;j++) { 
  for (i=0;i<10;i++) {
    if (i>4 && i<8) continue;
    c3++;
  }
  c3++;
}

result = (c1==7) && (c3==10+7*10);
