X1=-2.0;Y1=-2.0;X2=2.0;Y2=2.0;for (y=0;y<32;y++) {line="";for (x=0;x<32;x++) {Xr=0;Xi=0; Cr=X1+((X2-X1)*x/32);Ci=Y1+((Y2-Y1)*y/32);i=0;while ((i<32) && ((Xr*Xr+Xi*Xi)<4)) {t=Xr*Xr - Xi*Xi + Cr;Xi=2*Xr*Xi+Ci;Xr=t;i++;}if(i&1)line+="*";else line+=" "; } print(line);}

