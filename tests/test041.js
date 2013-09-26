// test break

var c1=0;
var c2=0;
var c3=0;

for (i=0;i<10;i++) {
  if (i>4) break;
  c1++;
}

for (i=0;i<10;i++) {
  c2++;
  if (i>4) break;
}

for (j=0;j<10;j++) { 
  for (i=0;i<10;i++) {
    if (i>4) break;
    c3++;
  }
  c3++;
}

result = (c1==5) && (c2==6) && (c3==10+5*10);
