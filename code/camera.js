var x = 0;
var step = function () {
  x++;
  digitalPulse(D0,1,1.25+(x/320.0));digitalPulse(D1,1,1.8-(y/40.0));
  if ((x>20) && ((x&1)!=0)) str+=getCol();
  if (x>160) { print(str);str="";
    x=0;
    y++;
    if (y>25) clearInterval(stepInterval);
  }
};
var cols = " .,-~:;=!*#$@";
var rangeMin = 0.414068;
var rangeMax = 0.52;
var getCol = function () {
 var a = analogRead(A1);if (a>max)max=a;if (a<min)min=a;
 a = cols.length*(a-rangeMin)/(rangeMax-rangeMin);
 if (a<0) a=0;
 if (a>=cols.length) a=cols.length-1;
 return cols.charAt(a);
};
var start = function () {
  x=0;
  y=0;str="";min=1;max=0;
  stepInterval = setInterval(step,25);
};
var stepInterval = 0;
var y = 26;
var str = "";
var stop = function () { clearInterval(stepInterval);stepInterval=undefined; };
var startOrStop = function () { if (stepInterval==undefined)
 start();
else
 stop();
};
var min = 0.4065;
var max = 0.516121;
