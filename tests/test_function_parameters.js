a = 5;
b = 6;
c = 7;
d = 8;
var args;
function x(a,b,c,d) {
  console.log(a,b,c,d,arguments);
  args = [a,b,c,d];
};

x(0);

result = args[0]===0 && args[1]===undefined  && args[2]===undefined  && args[3]===undefined;
