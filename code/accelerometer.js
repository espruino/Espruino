// Accelerometer on F4:
// init - see LIS302DL_CTRL_REG1_ADDR
SPI1.send([0x20,0b01000111], E3);

function getAcc() {
  var accx = SPI1.send([0xA9,0], E3)[1];
  var accy = SPI1.send([0xAB,0], E3)[1];
  if (accx>127) accx-=256;
  if (accy>127) accy-=256;
  analogWrite(LED2, accx/128.0);
  analogWrite(LED4, -accx/128.0);
  analogWrite(LED1, accy/128.0);
  analogWrite(LED3, -accy/128.0);
}

setInterval(getAcc, 100);


function onInit() {
  SPI1.send([0x20,0b01000111], E3);
}

var avrx=0.0, avry=0.0;
function getAcc() {
  var accx = SPI1.send([0xA9,0], E3)[1];
  var accy = SPI1.send([0xAB,0], E3)[1];
  if (accx>127) accx-=256;
  if (accy>127) accy-=256;
  avrx = 0.1*accx + 0.9*avrx;
  avry = 0.1*accy + 0.9*avry;
  analogWrite(LED2, avrx/128.0);
  analogWrite(LED4, -avrx/128.0);
  analogWrite(LED1, avry/128.0);
  analogWrite(LED3, -avry/128.0);
}
onInit();setInterval(getAcc, 10);