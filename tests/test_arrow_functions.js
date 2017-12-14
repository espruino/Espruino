// right now, not all of arrow functions are supported
// no destructuring/other ES6 stuff that isn't supported elsewhere

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

t( a=>42, 42);
t( ()=>42, 42);
t( (a)=>a*42, 42);
t( (a,b)=>a+b, 1+2);
t( (a,b,c)=>a+b+c, 1+2+3);

result = pass==tests;
