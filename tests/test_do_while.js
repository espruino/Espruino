var x=1;
do { x*=2; } while (x<255);


// http://forum.espruino.com/conversations/351365
var r="";
var idx = 0;
do {
  console.log(idx);
  r += idx;
  ++idx;
} while(idx <  4);

idx=0;
do {
  console.log(idx);
  r += idx;
} while(++idx <  4 );

result = x==256 && r=="01230123";

