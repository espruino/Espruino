// Espruino bluetooth voice activation
var command = "";

Serial3.onData(function(e) {
  command += e.data;
  if (e.data==" " || e.data=="\n") {
    command="";
  } else {
    print(command);
    if (command=="red") LED1.set();
    if (command=="green") LED2.set();
    if (command=="blue") LED3.set();
    if (command=="off") { LED1.reset(); LED2.reset(); LED3.reset(); }
  }
});

