// https://github.com/espruino/Espruino/issues/380

var tests=0, pass=0;

function cmp(ae, mustHave, maybeHave) {
  tests++;
  var a = eval(ae);
  var i;
  var fail = undefined;
  for (i in a) if (mustHave.indexOf(a[i])<0 && maybeHave.indexOf(a[i])<0) fail = "Shouldn't have '"+a[i]+"'";
  for (i in mustHave) if (a.indexOf(mustHave[i])<0) fail = "Missing '"+mustHave[i]+"'";
  if (fail) { console.log(ae, "Failed", fail, a, "vs", mustHave, "optional", maybeHave); return; }
  pass++;
}

cmp("Object.getOwnPropertyNames([])", [ 'length' ], [
])
cmp("Object.getOwnPropertyNames(Array.prototype)",
[ 
  'fill', // ES6
  'constructor',
  'toString',
  'join',
  'pop',
  'push',
  'concat',
  'shift',
  'unshift',
  'slice',
  'splice',
  'sort',
  'filter',
  'forEach',
  'some',
  'every',
  'map',
  'indexOf',
  'reduce'], [
  'length', // unsure why length has to be in proto
  'toLocaleString',
  'reduceRight',
  'lastIndexOf',
  'reverse',
]);
cmp("Object.getOwnPropertyNames(Array)",         
[ 'prototype',
  'isArray' ], [
  'length',
  'name',
  'arguments',
  'caller',
]);

result = tests==pass;
