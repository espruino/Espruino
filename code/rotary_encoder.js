var step = 0;
var bigStep = 0;
var r = require("Encoder").connect(A1,A2,function (s) {
  step-=s;
  while (step>40) step-=40;
  if (step<0) step=0;
  bigStep = step>>2;
  print(bigStep);
})

var code = [8,1,9,2];
var codeStep = 0;

function onClick() {
  if (codeStep<code.length && code[codeStep] == bigStep) {
    codeStep++;
    if (codeStep >= code.length) {
      print("UNLOCKED");
      digitalWrite([LED1,LED2,LED3], 0b010);
    } else {
      print("STEP "+codeStep);
    }
  } else {
    codeStep = 0;
    print("WRONG");
    digitalWrite([LED1,LED2,LED3], 0b100);
  }
}

pinMode(B12, "input_pulldown");
setWatch(onClick, B12, { repeat: true, edge: "rising" });
