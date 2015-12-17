// test for a8b6285cd8e5d65acc230ad47bec492e893cbe43

/* While refs will end up being big on all systems if this fails,
it will only break on 12 byte systems with JSVAR_CACHE_SIZE < 1024  
as that's when the ref counter is 8 bits and will overflow. */

a = undefined;
var i;
var oops = "Hello";
for (i=0;i<1040;i++) {
  a = { };
  a.a = a;
  a.b = oops;
  a = undefined;
  process.memory(); // force GC
}

trace(oops);
console.log("Refs ^^^^ should be something pretty small");

result = oops == "Hello";

