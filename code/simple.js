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


