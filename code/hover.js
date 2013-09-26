function onInit() {
  A0.set();
  A2.reset();
}
onInit();

function step() {
  a = analogRead(A1);
  d1=0;
  d2=0;
  if (a>b) {
    d1 = (a-b)/c;
  } else {
    d2 = (b-a)/c;
  }
  analogWrite(A6, d1);
  analogWrite(A7, d2);
  analogWrite(LED3, d1);
  analogWrite(LED2, d2);
}

setInterval(step,10);
b = 0.18;
c = 0.05;


