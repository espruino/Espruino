for (y=0;y<32;y++) {
  line="";
  for (x=0;x<32;x++) {
    var Xr=0;
    var Xi=0;
    var Cr=(4.0*x/32)-2.0;
    var Ci=(4.0*y/32)-2.0;
    var i=0;
    while ((i<8) && ((Xr*Xr+Xi*Xi)<4)) {
      var t=Xr*Xr - Xi*Xi + Cr;
      Xi=2*Xr*Xi+Ci;
      Xr=t;
      i++;
    }
    if (i&1)
	line += "*";
    else
        line += " ";
   }
 //  print(line);
}

