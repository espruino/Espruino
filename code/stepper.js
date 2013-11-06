// Simple stepper
var step = 0;
var steps = [0b0001,0b0010,0b0100,0b1000];
var stepperPins = [D9,D11,D14,D12];

function doStep() {
 step = (step+1) % steps.length;
 digitalWrite(stepperPins, steps[step]);
}
setInterval(doStep, 200);


// Stepper with dynamic speed
var step = 0;
var targetStep = 0;
var steps = [0b0001,0b0011,0b0010,0b0110,0b0100,0b1100,0b1000,0b1001];
var stepperPins = [D9,D11,D14,D12];
var stepInterval = setInterval(doStep, 100);
var doStep = function () {
 var d = step - targetStep;
 if (d < 0)
   step++;
 else if (d > 0)
   step--;
 if (d==0) { // we're there - sleep
   changeInterval(stepInterval, 500);
   digitalWrite(stepperPins, 0);
 } else {
   var time = 100 - Math.abs(d)*4;
   if (time<10) time=10;
   changeInterval(stepInterval, time);
   digitalWrite(stepperPins, steps[step%steps.length]);
 }
};

var targetStep = 0;
var stepInterval = setInterval(doStep, 100);

// Then run this to enable microstepping
var doStep = function () {
 var d = step - targetStep;
 if (d < 0)
   step++;
 else if (d > 0)
   step--;
 if (d==0) { // we're there - sleep
   changeInterval(stepInterval, 500);
   digitalWrite(stepperPins, 0);
 } else {
   var time = 100 - Math.abs(d)*4;
   if (time<50) time=50;
   changeInterval(stepInterval, time);
   analogWrite(stepperPins[0], Math.sin(0.5*step));
   analogWrite(stepperPins[1], Math.sin((0.5*step) + Math.PI*0.5));
   analogWrite(stepperPins[2], Math.sin((0.5*step) + Math.PI));
   analogWrite(stepperPins[3], Math.sin((0.5*step) + Math.PI*1.5));
 }
};
