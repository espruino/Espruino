// test for code that assert failed

var count = 0;

var moveServo = function (p) {
 var pulses = 20;
 var f = function() { count++; if (pulses-->0) setTimeout(f, 1); };
 f();
};

moveServo(1);
setTimeout("result=count==21;", 100);

