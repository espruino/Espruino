// test switch

var c = [];
var r=0;
for (i=0;i<7;i++) {
  c[i]=0;
  if (i<6) switch (i) {
   case 0: break;
            c[i]+=2312;
   case 1: 
   case 2:
            c[i]+=4;
   case 3: c[i]+=8;
            break;
   case 4: c[i]+=16;
           break;
   default: c[i]+=32; 
  }
  r++;
}

result = c[0]==0 &&c[1]==4+8 && c[2]==4+8 && c[3]==8 && c[4]==16 && c[5]==32 && c[6]==0 && r==7;
