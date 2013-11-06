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
      var t=Xr*Xr - Xi*Xi + Cr;
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
      var t=Xr*Xr - Xi*Xi + Cr;
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
   print(line);
}

y=0;
setInterval(function() { line="";  for (x=0;x<64;x++) {Xr=0;Xi=0; Cr=(4.0*x/64)-2.0;Ci=(4.0*y/64)-2.0;i=0;while ((i<32) && ((Xr*Xr+Xi*Xi)<4)) {t=Xr*Xr - Xi*Xi + Cr;Xi=2*Xr*Xi+Ci;Xr=t;i++;}if (i&1)line += "*";else line += " ";    }print(line); y++; }, 1000);

digitalWrite("C9", 1);
digitalWrite("C9", 0);

setWatch(function() {
  if (digitalRead("A0")) {
    digitalWrite("C9",1);
    setTimeout(function() {
      digitalWrite("C9", 0);
      setTimeout(function() {
        digitalWrite("C9", 1);
        setTimeout(function() {
          digitalWrite("C9", 0);
        }, 301);
      }, 302);
    }, 303);
}}, "A0", true);

setWatch(function() {
  if (digitalRead("A0")) {
    digitalWrite("C9",1);
    setTimeout(function() {
      digitalWrite("C9", 0);
    }, 303);
}}, "A0", true);

setWatch(function() { print(getTime()); }, "A0", true);

digitalPulse("C9",1,1000);

print("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
print("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
print("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
print("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

Serial1.onData(function(e){print(e.data);});



// LCD mandel
for (y=0;y<240;y++) {
  for (x=0;x<320;x++) {
    var Xr=0;
    var Xi=0;
    var Cr=(4.0*x/240)-2.0;
    var Ci=(4.0*y/240)-2.0;
    var i=0;
    while ((i<5) && ((Xr*Xr+Xi*Xi)<4)) {
      var t=Xr*Xr - Xi*Xi + Cr;
      Xi=2*Xr*Xi+Ci;
      Xr=t;
      i++;
    }
    LCD.setPixel(x,y, (i&1)?0xFFFFFF:0);
   }
}