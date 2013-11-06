for (y=0;y<128;y++) {
  line="";
  for (x=0;x<128;x++) {
    Xr=0;
    Xi=0;
    Cr=(4.0*x/128)-2.0;
    Ci=(4.0*y/128)-2.0;
    iterations=0;
    while ((iterations<128) && ((Xr*Xr+Xi*Xi)<4)) {
      t=Xr*Xr - Xi*Xi + Cr;
      Xi=2*Xr*Xi+Ci;
      Xr=t;
      iterations++;
    }
    if (iterations&1)
	line += "*";
    else
        line += " ";
   }
   print(line);
  }

// time ./TinyJSC test ../code/mandel-big.js
// user	0m8.897s - 6/8/2012
// user	0m8.501s - 6/8/2012 - make jsvIs* inline
// user	0m7.160s - 6/8/2012 - no asserts
// user	0m6.60s - 6/8/2012 - add jsvLockAgain instead of lock/getRef
