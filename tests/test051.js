// test that interrupt works 2

var result = -5;
for(j=0;j<10;j++) {
  for(i=0;i<10;i++) {
    result++;
    if (i>4) interrupt();
  }
}
result=0;
