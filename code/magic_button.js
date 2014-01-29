var inAnim = false;
var pos = 0;
var anim = [
//[pointer, arm],
[1,0],  // folded away
[0.9,0.0], // touching the lid
[0.8,0.05], // push up
[0.7,0.2],
[0.6,0.3],
[0.5,0.5],
[0.3,0.7],
[0.1,1],
[0.25,1], // press
[0,1],
[0.3,0.7],
[1,0],
];

var lastPress = 0;
function onPress(e) {
  if (e.time < lastPress + 0.5) return;
  lastPress = e.time;
  if (inAnim) {
    C6.reset(); // light off
  } else {
    // startanim
    C6.set(); // light on
    inAnim = true;
    pos = 0;
    print("interval "+setInterval(onAnimStep, 25));
  }
}
function onAnimStep() {
  pos += 0.02;
  if (pos > anim.length) {
    clearInterval(0);
    digitalWrite([LED1,LED2,LED3], 0); // off status
    inAnim = false;
    return;
  }
  analogWrite(LED1, pos);
  analogWrite(LED2, pos-1);
  analogWrite(LED3, pos-2);
  var i = pos|0;
  var f = pos-i;
  if (i>anim.length-2) {
    i=anim.length-2;
    f=1;
  }
  digitalPulse(B12, 1, 2-E.clip(anim[i][0]*(1-f) + anim[i+1][0]*f,0,1));
  digitalPulse(B13, 1, 2-E.clip(anim[i][1]*(1-f) + anim[i+1][1]*f,0,1));
}
setWatch(onPress, B15, { repeat:true, edge:'falling' });


s = [1,0];
setInterval("digitalPulse(B12, 1, 2-s[0]);digitalPulse(B13, 1, 2-s[1]);", 50);
clearInterval();

