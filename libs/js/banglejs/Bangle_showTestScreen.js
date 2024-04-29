(function() {
  Bangle.setUI();clearInterval();clearWatch();g.clear(1);
  Bangle.removeAllListeners();E.removeAllListeners();
  Bangle.setBarometerPower(1, "app");
  Bangle.setCompassPower(1, "app");
  Bangle.setGPSPower(1, "app");
  Bangle.setHRMPower(1, "app");
  Bangle.setLCDPower(1);
  Bangle.setLocked(0);
  Bangle.setLCDTimeout(0);
  var pass = [];
  const abs = Math.abs;
  function draw(n,id,s,ok) {
    pass[n]=ok;
    var y = n*19;
    g.reset().clearRect(48,y,175,y+19).setFont("6x8:1x2");
    g.setBgColor(ok ? "#0f0" : "#f00").clearRect(0,y,48,y+19);
    g.setFontAlign(0,0).drawString(id, 24, y+10);
    g.setFontAlign(-1,0).drawString(s, 52, y+10);
  }
  "TS,GPS,HRM,Baro,Mag,Acc,Btn,FW,Chg".split(",").forEach((e,i)=>draw(i,e,"",false));
  Bangle.on('touch',(n,e)=>draw(0,"TS",e.x+","+e.y, e.x>-25 && e.y>-25 && e.x<200 && e.y<200));
  Bangle.on('GPS',fix=>draw(1,"GPS",fix.time?require("locale").time(fix.time,1):"--",true));
  Bangle.on('HRM-raw',h=>draw(2,"HRM",h.vcPPG,h.vcPPG>=0 && h.vcPPG<=5000));
  Bangle.on('pressure',p=>draw(3,"Baro",Math.round(p.pressure), p.pressure>900 && p.pressure<1100));
  Bangle.on('mag',p=>draw(4,"Mag",p.x+","+p.y+","+p.z, abs(p.x)<5000 && Math.abs(p.y)<5000 && Math.abs(p.z)<5000));
  Bangle.on('accel',p=>draw(5,"Acc",p.x.toFixed(1)+","+p.y.toFixed(1)+","+p.z.toFixed(1), abs(p.x)<0.5 && abs(p.y)<0.5 && p.z<-0.8 ));
  setWatch(e=>draw(6,'Btn',e.state?"Press":"Released",true),BTN,{edge:0,repeat:1});
  var bootcode=require("Storage").read(".bootcde");
  draw(7,'FW',"boot "+(require("Storage").readJSON("boot.info",1)||{}).version||"NONE", bootcode&&bootcode.includes("clockHasWidgets"));
  setTimeout(function() {
    var mv = (0.0001+analogRead(D3)+analogRead(D3)+analogRead(D3))/3, cup = 0, chg = 0;
    setInterval(function() {
      var v = analogRead(D3);
      chg |= Bangle.isCharging();
      if (v>mv) cup=1;
      draw(8,'Chg',(v*13.359).toFixed(2)+"v "+(Bangle.isCharging()?"charge":"discharge"),chg&&cup);
    },1000);
  },1000);
  Bangle.on('swipe', e => {
    if (!pass.every(a=>a)) return;
    Bangle.removeAllListeners();
    clearInterval();clearWatch();
    NRF.sleep(); // Bluetooth off
    Bangle.setBarometerPower(0, "app");
    Bangle.setCompassPower(0, "app");
    Bangle.setGPSPower(0, "app");
    Bangle.setHRMPower(0, "app");
    Bangle.setBacklight(0);
    Bangle.setLocked(1); // touchscreen off
    g.clear(1).setFont("12x20:2").setFontAlign(0,0).drawString("TEST\nPASS",88,88);
    require('Storage').writeJSON('welcome.json', {welcomed: false});
    Bangle.setPollInterval(800); // force low power accelerometer mode
    setTimeout(function() {
      Bangle.softOff();
    }, 60*60*1000); // 1 hour
  });
})