CRANK=A0;
INL=A1;
EXH=A2;


var tooth = 0;
var toothOn = false;
function doTooth() {
  toothOn = !toothOn;
  CRANK.write(toothOn && tooth<58);
  if (!toothOn) {
    tooth++;
    if (tooth>59) tooth=0;
    LED1.write(tooth==0);
  }
}

setInterval(doTooth, 10);
//changeInterval(doTooth, 10); // assert fail?

// -----------------------------------------------------------------
var CRANK = B12;
var INL = D14;
var EXH = D8;

var CRANK = A0;
var INL = A1;
var EXH = A2;
var tooth = 4;
var toothOn = false;
var inletPos = 35.4;
var msPerTooth = 8;

function doTooth() {
  toothOn = !toothOn;
  CRANK.write(toothOn && tooth<58);
  if (!toothOn) {
    tooth++;
    if (tooth>59) tooth=0;
    LED1.write(tooth==0);
    if ((0|inletPos)==tooth) {
      setTimeout("digitalPulse(INL, 0, 10);", msPerTooth*(inletPos - (0|inletPos)));
    }
  }
}
function update() {
  inletPos = analogRead(C0)*30;
}
var doToothInterval = setInterval(doTooth, 4);
var updateInterval = setInterval(update, 200);

function setRPM(rpm) {
  msPerTooth = 1000/rpm;
  changeInterval(doToothInterval, msPerTooth/2);
}

