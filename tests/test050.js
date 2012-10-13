// test that interrupt works

var a=0;
for(i=0;i<10;i++) {
  a++;
  if (i>4) interrupt();
}

var b = 0;
for(j=0;j<10;j++) {
  for(i=0;i<10;i++) {
    b++;
    if (i>4) interrupt();
  }
}
 
result =  a==6 && b==6;
