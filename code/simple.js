function foo(a,b) { return a+333*b;};foo(1,4);

function flash(c) {
  if (c<=0) return;
  setTimeout(function() {
    digitalWrite("C9",1);
    setTimeout(function() {
      digitalWrite("C9",0);
      flash(c-1);
    }, 250);
  }, 250);
}

var count = 0;
setWatch(function() {
 if (!digitalRead("A0")) return;
 flash(count++);
}, "A0", true);



var count = 0;
setWatch(function () {
 if (!digitalRead("A0")) return;
 print(count);
 count++;
}, "A0", 1);

// measure the time between button presses
var downTime = 0;
setWatch(function(x) {
  if (digitalRead("A0"))
    downTime = x.time;
  else
    print(x.time - downTime);
}, "A0", true);

// flash 4 lights on stm32f4 around
var n=1;
setInterval(function () {
 n=n>>1;
 if (n==0) n=16;
 digitalWrite(["D12","D13","D14","D15"],n);
},50);


// just write different analog values...
analogWrite("D12",0.5);
analogWrite("D13",0.25);
analogWrite("D14",0.75);
analogWrite("D15",1);

var ramp = [0,0.05,0.2,0.3,0.5,1,0.5,0.3,0.2,0.05];
var c = 0;
var d = 2;
var e = 4;
var f = 8;
function next() {
  c = (c+1)%ramp.length;
  d = (d+1)%ramp.length;
  e = (e+1)%ramp.length;
  f = (f+1)%ramp.length;
  analogWrite("D12",ramp[c]);
  analogWrite("D13",ramp[d]);
  analogWrite("D14",ramp[e]);
  analogWrite("D15",ramp[f]);
}
setInterval(next,50);


analogWrite("C9",0.1);

analogWrite("D12",0.5);
analogWrite("D13",0.25);
analogWrite("D14",0.75);
analogWrite("D15",1);

var ramp = [0,0.05,0.2,0.3,0.5,1,0.5,0.3,0.2,0.05];
var c = 0;
var d = 5;
function next() {
  c = (c+1)%ramp.length;
  d = (d+1)%ramp.length;
  analogWrite("C8",ramp[c]);
  analogWrite("C9",ramp[d]);
}
setInterval(next,50);


// measure time between keypresses and set light brightness
var bright = 0;
var target = 0;
var lastPress = 0;
setWatch(function(e) {
 if (digitalRead("A0"))
   lastPress = e.time;
 else
   target = (e.time-lastPress)/2;
}, "A0",true);
setInterval(function() {
  bright=bright*0.9+target*0.1;
  analogWrite("C9",bright-0.1);
  analogWrite("C8",bright-0.6);
},100);



// speed test
var a = 1;
setInterval("a=!a;digitalWrite('D12',a);", 0);
var b = 1;
setInterval("b=!b;digitalWrite('D13',b);", 0);

// servo test
var coords = [0.876,0.5];
var step = function () {
  t+=s;
  if (t>1) t=0;
  if (t<0.25) {
    coords = [t*4,0];
  } else if (t<0.5) {
    coords = [1,(t-0.25)*4];
  } else if (t<0.75) {
    coords = [1-((t-0.5)*4),1];
  } else {
    coords = [0,1-((t-0.75)*4)];
  }
};
var step = function () {
 t+=s;
 if (t>1) t=0;
 coords = [0.5 + Math.sin(t*2*Math.PI)*0.5, 0.5+Math.cos(t*2*Math.PI)*0.5];
 print(coords[0]+","+coords[1]);
}


var pulse = function () {
  digitalPulse(D0,1,1+coords[0]);
  digitalPulse(D1,1,1+coords[1]);
}
var t = 0.438;
var s = 0.001;
setInterval("step();pulse();", 50);



var test = function () {
 digitalWrite(A0,0);digitalWrite(A2,1); average=average*0.9 + 0.1*analogRead(A1);
 if (!on && average<0.4) setOn();
 if (on && average>0.5) setOff();
};
var on = 1;
var setOn = function () { on=true; digitalWrite(LED1,1); setTimeout("digitalWrite(LED1,0)",1000);};
var setOff = function () { on=false; digitalWrite(LED2,1); setTimeout("digitalWrite(LED2,0)",1000);};
var average = 0.414431;
setInterval("test()", 100);


// scroller
var leds = [D9,D11,D12,D14];
var state = 10307921510;
var scroll = function () {
  state = ((state>>1)&0b0111) | (state<<3);
  digitalWrite(leds, state);
};




// ----------------------------------- All 4 lights
var c = 0;
var next = function () {
  c+=0.1;
  analogWrite(LED1,(Math.sin(c)+1)*0.15);
  analogWrite(LED2,(Math.sin(c+Math.PI)+1)*0.15);
};
setInterval(next, 50);


var c = 0;
var next = function () {
  c+=0.1;
  analogWrite("D12",(Math.sin(c)+1)*0.25);
  analogWrite("D13",(Math.sin(c+Math.PI*0.5)+1)*0.25);
  analogWrite("D14",(Math.sin(c+Math.PI)+1)*0.25);
  analogWrite("D15",(Math.sin(c+Math.PI*1.5)+1)*0.25);
}
setInterval(next, 50);

