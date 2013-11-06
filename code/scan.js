var servox = D6;
var servoy = D5;

function start() {
 x=0;
 y=0;
 dir=1;
 interval = setInterval(step, 50);
}
function stop() {
  clearInterval(interval);
}
function step() {
 x+=dir;
 var lastDir = dir;
 if (dir>0 && x>40) dir=-1;
 if (dir<0 && x<1) dir=1;
 if (dir!=lastDir) {
   print(str);
   str="";
   y++;
   if (y>20) stop();
 } else {
   if (dir>0) str=str+get();
   else str=get()+str;
 }
 digitalPulse(servox,1,1+(x/40.0));
 digitalPulse(servoy,1,1+(y/20.0));
}
function get() { return ':'; }
