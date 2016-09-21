var pass = 0;
var tests = 0;

function cmp(a,b) {
  tests++;
  if (a==b) return pass++;
  console.log(JSON.stringify(a),"!=",JSON.stringify(b));
  return 0;
}
var w = "world";

cmp(`Hello`,"Hello");
cmp(`He$lo`,"He$lo");
cmp(`Hell${0} world`,"Hell0 world");
cmp(`H${1+2}ll${0} ${w}`,"H3ll0 world");
cmp(`Its an ${{}}`,"Its an [object Object]");

result = pass==tests;
