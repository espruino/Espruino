// test that interrupt works

var result=-5;
for(i=0;i<10;i++) {
  a++;
  if (i>4) interrupt();
}
result=0;
