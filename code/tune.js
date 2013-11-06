var tune = "g g C.. C.. g g C.. C.. D D E... E... D D E.. E.. G.. G....    G.. G....  G F E.. E.. G F E.. E.. ";
var tune = "g e    g    g   e    g   A  A  g f  e  d   e   f";
var tune = "c    c    c   d    e   e  d   e    f   g   C  C C   g  g g   e  e e   c  c c  g    f  e   d c";
var pos = 0;
var BUZZER=A1;
function step() {
  var ch = tune[pos];
  if (ch!=undefined) pos++;
  if (ch==' ' || ch==undefined) freq(0); // off
  else if (ch=='a') freq(220.00);
  else if (ch=='b') freq(246.94);
  else if (ch=='c') freq(261.63);
  else if (ch=='d') freq(293.66);
  else if (ch=='e') freq(329.63);
  else if (ch=='f') freq(349.23);
  else if (ch=='g') freq(392.00);
  else if (ch=='A') freq(440.00);
  else if (ch=='B') freq(493.88);
  else if (ch=='C') freq(523.25);
  else if (ch=='D') freq(587.33);
  else if (ch=='E') freq(659.26);
  else if (ch=='F') freq(698.46);
  else if (ch=='G') freq(783.99);
}
function freq(freq) {
  if (freq==0) digitalWrite(BUZZER,0);
  else analogWrite(BUZZER, 0.5, { freq: freq } );
}
setInterval(step, 200);
