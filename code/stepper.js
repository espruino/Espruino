// Simple stepper
var step = 0;
var steps = [1,3,2,6,4,12,8,9];
var stepperPins = ["D9","D11","D14","D12"];

function doStep() {
 step = (step+1) % steps.length;
 digitalWrite(stepperPins, steps[step]);
}
setInterval(doStep, 200);


// Stepper with dynamic speed
var step = 0;
var steps = [1,3,2,6,4,12,8,9];
var stepperPins = ["D9","D11","D14","D12"];
var doStep = function () {    
 var d = step - targetStep;
 if (d < 0) 
   step++;                             
 else if (d > 0)
   step--;
 if (d==0) { // we're there - sleep
   changeInterval(stepperInterval, 500);
   digitalWrite(stepperPins, 0);
 } else {
   var time = 100 - Math.abs(d)*4;
   if (time<10) time=10; 
   changeInterval(stepperInterval, time);
   digitalWrite(stepperPins, steps[step%steps.length]);
 }
};
var targetStep = 0;
var Math = function () {};
var stepperInterval = setInterval(doStep, 100);
