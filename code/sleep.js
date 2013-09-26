setWatch(function() {
 LED1.set();
 setTimeout("LED1.reset();LED2.set();",500);
 setTimeout("LED2.reset();LED3.set();",1000);
 setTimeout("LED3.reset()",1500);
}, BTN, {repeat: true, edge:"rising"});

setDeepSleep(1);
