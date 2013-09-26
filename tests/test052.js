// test that interrupt works 3

var result = -5;
for(j=0;j<10;j++) {
  i=0;
  while (i++ < 10) {
    result++;
    if (i>5) interrupt();
  }
}
result=0;
