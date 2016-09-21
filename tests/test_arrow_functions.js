// right now, only simple arrow functions are supported
// so no (a)=>a type stuff

var pass = 0;
var tests = 0;

function t(a,b) {
  tests++;
  var r = a(1,2,3);
  if (r==b) pass++;
  else console.log(r,"!=",b);
}


t( (a)=>{return a*42;}, 42);
t( (a,b)=>{return a+b;}, 1+2);
t( (a,b,c)=>{return a+b+c;}, 1+2+3);

result = pass==tests;
