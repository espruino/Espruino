/* Mandelbrot! */

for (y=0;y<32;y++) {
  line="";
  for (x=0;x<32;x++) {
    Xr=0;
    Xi=0; 
    Cr=(4.0*x/32)-2.0;
    Ci=(4.0*y/32)-2.0;
    iterations=0;
    while ((iterations<32) && ((Xr*Xr+Xi*Xi)<4)) {
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

//smaller
for (y=0;y<16;y++) {
  line="";
  for (x=0;x<16;x++) {
    Xr=0;
    Xi=0; 
    Cr=(4.0*x/16)-2.0;
    Ci=(4.0*y/16)-2.0;
    iterations=0;
    while ((iterations<16) && ((Xr*Xr+Xi*Xi)<4)) {
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

for (y=0;y<32;y++) {  line="";  for (x=0;x<32;x++) {Xr=0;Xi=0; Cr=(4.0*x/32)-2.0;Ci=(4.0*y/32)-2.0;i=0;while ((i<32) && ((Xr*Xr+Xi*Xi)<4)) {t=Xr*Xr - Xi*Xi + Cr;Xi=2*Xr*Xi+Ci;Xr=t;i++;}if (i&1)line += "*";else line += " ";    }print(line);}

for (y=0;y<64;y++) {  line="";  for (x=0;x<64;x++) {Xr=0;Xi=0; Cr=(4.0*x/64)-2.0;Ci=(4.0*y/64)-2.0;i=0;while ((i<32) && ((Xr*Xr+Xi*Xi)<4)) {t=Xr*Xr - Xi*Xi + Cr;Xi=2*Xr*Xi+Ci;Xr=t;i++;}if (i&1)line += "*";else line += " ";    }print(line);}
