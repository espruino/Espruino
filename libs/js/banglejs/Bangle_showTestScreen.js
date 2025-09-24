(function() {
  Bangle.setUI();clearInterval();clearWatch();g.clear(1);
  Bangle.removeAllListeners();E.removeAllListeners();
  Bangle.setPollInterval(80);
  Bangle.setBarometerPower(1, "app");
  Bangle.setCompassPower(1, "app");
  Bangle.setGPSPower(1, "app");
  Bangle.setHRMPower(1, "app");
  Bangle.setLCDPower(1);
  Bangle.setLocked(0);
  Bangle.setLCDTimeout(0);
  var pass = [];
  const TESTS = "TS,GPS,HRM,Baro,Mag,Acc,Btn,FW,Chg,Vibrate".split(",");
  const abs = Math.abs;
  var vibOn = false, vibCounter = 0, vibMotion;
  function draw(id,s,ok) {
    var n = TESTS.indexOf(id);
    pass[n]=ok;
    var y = n*17;
    g.reset().clearRect(48,y,175,y+17).setFont("6x8:1x2");
    g.setBgColor(ok ? "#0f0" : "#f00").clearRect(0,y,48,y+17);
    g.setFontAlign(0,0).drawString(id, 24, y+9);
    g.setFontAlign(-1,0).drawString(s, 52, y+9);
  }
  TESTS.forEach(id=>draw(id,"",false));
  Bangle.on('touch',(n,e)=>draw("TS",e.x+","+e.y, e.x>-25 && e.y>-25 && e.x<200 && e.y<200));
  Bangle.on('GPS',fix=>draw("GPS",fix.time?require("locale").time(fix.time,1):"--",true));
  Bangle.on('HRM-raw',h=>draw("HRM",h.vcPPG,h.vcPPG>=0 && h.vcPPG<=5000));
  Bangle.on('pressure',p=>draw("Baro",Math.round(p.pressure), p.pressure>900 && p.pressure<1100));
  Bangle.on('mag',p=>draw("Mag",p.x+","+p.y+","+p.z, abs(p.x)<5000 && Math.abs(p.y)<5000 && Math.abs(p.z)<5000));
  Bangle.on('accel',p=>{
    draw("Acc",p.x.toFixed(1)+","+p.y.toFixed(1)+","+p.z.toFixed(1), abs(p.x)<0.5 && abs(p.y)<0.5 && p.z<-0.8 );
    if (vibOn) { // if testing, wait for 10 readings and sum difference to see if it moved
      vibCounter++;
      vibMotion += p.diff;
      if (vibCounter>10) {
        vibOn = false;
        D19.reset();
        var ok = vibMotion>3;
        draw("Vibrate",ok?"ok":"fail",ok);
      }
    } else if ((vibMotion===undefined) && p.diff < 0.12){
      // wait until the Bangle has been still for a few seconds before turning vib on
      vibCounter++;
      if (vibCounter>15) {
        vibCounter = 0;
        vibOn = true;
        vibMotion = 0;
        D19.set();
      }
    } else vibCounter = 0;

  });
  setWatch(e=>draw('Btn',e.state?"Press":"Released",true),BTN,{edge:0,repeat:1});
  var bootcode=require("Storage").read(".bootcde");
  draw('FW',"boot "+(require("Storage").readJSON("boot.info",1)||{}).version||"NONE", bootcode&&bootcode.includes("clockHasWidgets"));
  setTimeout(function() {
    var mv = (0.0001+analogRead(D3)+analogRead(D3)+analogRead(D3))/3, cup = 0, chg = 0;
    setInterval(function() {
      var av = analogRead(D3), v=av*13.359;
      chg |= Bangle.isCharging();
      if (av>mv) cup=1;
      var ok = chg&&cup&&(v>2)&&(v<4.4), msg = v.toFixed(2)+"v "+(Bangle.isCharging()?"charge":"discharge");
      if (!peek32(0x40000578)) { // DCDCEN
        msg = "NO DCDC";
        ok = false;
      }
      draw('Chg',msg,ok);
    },500);
  },1000);
  Bangle.on('swipe', e => {
    if (!pass.every(a=>a)) return;
    Bangle.removeAllListeners();
    clearInterval();clearWatch();
    g.clear();
    setTimeout(function() {
      Bangle.setBarometerPower(0, "app");
      Bangle.setCompassPower(0, "app");
      Bangle.setGPSPower(0, "app");
      Bangle.setHRMPower(0, "app");
      Bangle.setBacklight(0);
      Bangle.setLocked(1); // touchscreen off
      g.clear(1).setFont("12x20:2").setFontAlign(0,0).drawString("TEST\nPASS",88,88);
      require('Storage').writeJSON('welcome.json', {welcomed: false});
      Bangle.setPollInterval(800); // force low power accelerometer mode
      NRF.sleep(); // Bluetooth off
      setTimeout(function() {
        Bangle.off();
      }, 60*60*1000); // 1 hour
    }, 100);
  });
})
