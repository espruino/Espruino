// ctrl-c crashes it!

var PINS = {
  iCrank:C8,
  iInl:C10,
  iExh:C9,
  iSpare:C11,
  oInl:E8,
  oExh:E10,
  oDashECU:E9,
  oDashOil:E11,
  oDashTemp:E12,
  oDashRPM:E13,
  aBat:C5,
  aTPS:A3,
  aMAP:B0,
  aWater:C1,
  aAir:B1,
  aLambdaM:A2,
  aLambdaP:A1,
  oCoil1:D6,
  oCoil2:D7,
  oCoil3:D10,
  oCoil4:D11,
  oInj1:E3,
  oInj2:E4,
  oInj3:E5,
  oInj4:E6,
};
var ECU = {
"pInl":2.598771,
"pExh":43.764055,
"RPM":125.030327,
"TPS":0.260746,
"mapx":0.12503,
"mapy":1.607461,
"spk":10,
"spkStart":8.499636,
"vBat":16.765931,
"spkLen":2,
"expInl":20,
"expExh":20};

var MAPS = {
  spk : new Float32Array( [
//  0,   1,    2,  3,  4,  5,  6, 7, RPM
   5,  13,  13,  13,  18,  24,  27,  27,
   5,  13,  13,  13,  18,  24,  27,  27,
   5,  13,  13,  13,  20,  24,  27,  27,
   5,  14,  13,  20,  25,  26,  25,  25,
   5,  16,  20,  24,  24,  25,  25,  25,
   5,  15,  20,  23,  24,  24,  24,  24,
   5,   6,  18,  20,  21,  21,  21,  21,
   5,   6,  18,  20,  21,  21,  21,  21,
] ),
  fuel : new Float32Array( [
0x1C, 0x0F, 0x0E, 0x0B, 0x0C, 0x0C, 0x13, 0x12,
0x1C, 0x10, 0x0E, 0x0C, 0x16, 0x17, 0x23, 0x2A,
0x1C, 0x1F, 0x1B, 0x1E, 0x1D, 0x26, 0x32, 0x33,
0x2E, 0x31, 0x27, 0x24, 0x24, 0x30, 0x3F, 0x3D,
0x37, 0x3E, 0x2F, 0x35, 0x39, 0x49, 0x48, 0x46,
0x43, 0x45, 0x45, 0x4F, 0x55, 0x52, 0x4C, 0x44,
0x4D, 0x45, 0x51, 0x55, 0x58, 0x53, 0x4F, 0x48,
0x4D, 0x4C, 0x5A, 0x56, 0x55, 0x52, 0x4E, 0x48,
] ),
  inl : new Int8Array( [
    0,   0,   0,   0,   0,  -3, -3, -3, // le5
   -4,  -4, -13, -19,  -9,  -3, -3, -3,
   -2,  -2, -13, -17,  -6,  -2, -2, -2,
   -2,  -2, -13, -17,  -6,  -2, -2, -2,
  -24, -24, -24, -24, -24, -13, -7, -7,
  -24, -24, -24, -24, -24, -13, -7, -7,
  -24, -24, -24, -24, -24, -13, -7, -7,
  -24, -24, -24, -24, -24, -13, -7, -7,
] ),
  exh : new Int8Array( [
   0,   0,   5,   9,   9,  14,  14,  14, // le5
   0,  13,  24,  24,  19,  15,  15,  17,
   0,  13,  22,  18,  15,  12,  14,  16,
   0,   8,  22,  14,  10,  10,  12,  14,
   0,   7,  10,  12,  17,  11,  10,  10,
   0,   7,  10,  12,  17,  11,  10,  10,
   0,   7,  10,  12,  17,  11,  10,  10,
   0,   7,  10,  12,  17,  11,  10,  10,
] )
};


function watchInlet(e) {
  ECU.pInl = Math.wrap(Trig.getPosAtTime(e.time),180);
  var diff = ECU.pInl - ECU.expInl;
  ECU.drvInl = 0.8 + (diff / 10);
  analogWrite(PINS.oInl, ECU.drvInl);
}
function watchExhaust(e) {
  ECU.pExh = Math.wrap(Trig.getPosAtTime(e.time)+90,180)-90;
  var diff = ECU.expExh - ECU.pExh;
  ECU.drvExh = 0.8 + (diff / 10);
  analogWrite(PINS.oExh, ECU.drvExh);
}

// inl = 46 .. 96 (on)
// exh = -2.5(off) .. 47(on)

// NOTE: a degree value of -7.75*6 will occur on the first tooth after
function onTimer() {
  ECU.RPM = Trig.getRPM();
  ECU.TPS = analogRead(PINS.aTPS);
  ECU.MAP = E.clip((0.1449 / (1.125-analogRead(PINS.aMAP))) - 0.125,0,1);
  ECU.vBat = analogRead(PINS.aBat)*18.804;
  ECU.mapx = E.clip(ECU.RPM/1000, 0, 7);
  ECU.mapy = E.clip(ECU.TPS*10-1,0,8);
  ECU.spk = -MAPS.spk.interpolate2d(8,ECU.mapx, ECU.mapy);
  ECU.spkLen = 1.5 + E.clip((13.5-ECU.vBat)*0.1,0,0.5);
  ECU.expInl = 90+MAPS.inl.interpolate2d(8,ECU.mapx, ECU.mapy);
  ECU.expExh = MAPS.exh.interpolate2d(8,ECU.mapx, ECU.mapy);
  ECU.spkStart = ECU.spk - (ECU.RPM*ECU.spkLen*6/1000);
  ECU.fuel = MAPS.fuel.interpolate2d(8,ECU.mapx, ECU.mapy) * ECU.trim;
  Trig.setTrigger(0,ECU.spkStart,[LED1,PINS.oCoil1,PINS.oCoil4],ECU.spkLen);
  Trig.setTrigger(1,180+ECU.spkStart,[LED3,PINS.oCoil2,PINS.oCoil3],ECU.spkLen);
  Trig.setTrigger(2,0,[PINS.oInj1,PINS.oInj2,PINS.oInj3,PINS.oInj4],ECU.fuel);
  Trig.setTrigger(3,180,[PINS.oInj1,PINS.oInj2,PINS.oInj3,PINS.oInj4],ECU.fuel);
  ECU.lambda = 7+15.25*(analogRead(PINS.aLambdaP) - analogRead(PINS.aLambdaM));
  if (ECU.RPM>500 && ECU.lambda>7.5)
    ECU.trim = (ECU.trim*0.99) + (0.01*E.clip(ECU.trim*ECU.lambda/15,0.05,0.2));

 var a = Trig.getErrorArray();
  if (a.length>0) print(JSON.stringify(a));
}

function onInit() {
  Serial1.setup(9600, {"rx":B7,"tx":B6});
  Serial1.setConsole();
  clearInterval();
  setInterval(onTimer,25);
  clearWatch();
  Trig.setup(PINS.iCrank, { teethTotal:60, teethMissing:2, minRPM:30, keyPosition: -7.75*360/60 });
  setWatch(watchInlet, PINS.iInl, {repeat:true, edge:'falling'});
  setWatch(watchExhaust, PINS.iExh, {repeat:true, edge:'falling'});
  PINS.oCoil1.reset();
  PINS.oCoil2.reset();
  PINS.oCoil3.reset();
  PINS.oCoil4.reset();
  LED1.reset();
  LED2.reset();
  LED3.reset();
  LED4.reset();
  Trig.setTrigger(0,0,[LED1,PINS.oCoil1,PINS.oCoil4],5);
  Trig.setTrigger(1,180,[LED3,PINS.oCoil2,PINS.oCoil3],5);
  Trig.setTrigger(2,90,[LED2],5);
  Trig.setTrigger(3,270,[LED4],5);
}
onInit();

// 1.808 at 3000rpm idle

digitalPulse(PINS.oInj1,1,10);digitalPulse(PINS.oInj2,1,10);digitalPulse(PINS.oInj3,1,10);digitalPulse(PINS.oInj4,1,10);
// lambda
//0.183596 9.8
//0.354009 12.4
//(12.4-9.8)/(0.354009-0.183596)

E8.reset();Trig.setTrigger(0,10,[E8],100);
setInterval("print(JSON.stringify(ECU))",1000);

PINS.oCoil1.reset();
PINS.oCoil3.reset();
Trig.setTrigger(0,1,[PINS.oCoil1,PINS.oCoil3],5);
//---------------------------------------------------------
//E8.reset();Trig.setTrigger(0,10,[E8],100);

E8.reset();setInterval("var t=getTime();E8.writeAtTime(1,t+0.005);E8.writeAtTime(0,t+0.006);",100);

LED3.reset();setInterval("var t=getTime();LED3.writeAtTime(1,t);LED3.writeAtTime(1,t+1);LED3.writeAtTime(0,t+2);",1500);
LED4.reset();setInterval("var t=getTime();LED4.writeAtTime(1,t+1);LED4.writeAtTime(0,t+2);",1600);
LED3.reset();var t=getTime();LED3.writeAtTime(1,t+10);LED3.writeAtTime(0,t+11);LED3.writeAtTime(1,t+12);LED3.writeAtTime(0,t+13);
var t=getTime();LED3.writeAtTime(1,t+2);LED3.writeAtTime(0,t+3);

LED1.reset();LED2.reset();E8.reset();
var t=getTime();for (i=0;i<8;i++) E8.writeAtTime(i&1,t+i);

LED1.reset();LED2.reset();E8.reset();
var t=getTime();E8.writeAtTime(i&1,t+0);E8.writeAtTime(i&1,t+1);


LED1.reset();LED2.reset();
var t=getTime();LED3.write(1);for (i=0;i<8;i++) LED3.writeAtTime(i&1,t+i+1);

//---------------------------------------------------------


function step() {
//  print([C8.read(),C9.read(),C10.read(),C11.read()]);
}
setInterval(step,500);


       Trig.setup(BTN2);
        var x=0;
        function step() {
          var t = Trig.getPosAtTime(getTime());
          LCD.drawString(t+"    ",0,0,0,0xFFFF);
          LCD.setPixel(x, 240-t*3, 0xFFFF);
          x++;
          if (x>319) { x=0; LCD.clear(); }
        }

        setInterval(step, 50);


        ["MISSED_TRIG_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","SHORT_TOOTH"]
        ["SHORT_TOOTH"]
        ["WHEEL_MISSED_TOOTH","TRIG_TOOTH_CHANGED"]
        ["MISSED_TOOTH"]
        ["TRIG_IN_PAST","MISSED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","SHORT_TOOTH"]
        ["TRIG_IN_PAST","WHEEL_MISSED_TOOTH","WHEEL_MISSED_TRIG_TOOTH"]
        ["TRIG_IN_PAST","WHEEL_MISSED_TRIG_TOOTH"]
        ["TRIG_IN_PAST","WHEEL_MISSED_TRIG_TOOTH"]
        ["WHEEL_MISSED_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TOOTH","WHEEL_MISSED_TRIG_TOOTH"]
        ["TRIG_IN_PAST","WHEEL_MISSED_TOOTH","WHEEL_MISSED_TRIG_TOOTH"]
        ["TRIG_IN_PAST","WHEEL_MISSED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["WHEEL_MISSED_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","SHORT_TOOTH"]
        ["TRIG_IN_PAST","WHEEL_MISSED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","SHORT_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","WHEEL_MISSED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_FUTURE","TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_FUTURE","TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        ["TRIG_IN_FUTURE","TRIG_IN_PAST","MISSED_TRIG_TOOTH","WHEEL_MISSED_TOOTH","WHEEL_GAINED_TOOTH","WHEEL_MISSED_TRIG_TOOTH","TRIG_TOOTH_CHANGED"]
        >
