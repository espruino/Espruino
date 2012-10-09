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
