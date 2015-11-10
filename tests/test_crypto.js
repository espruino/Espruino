
ArrayBuffer.prototype.toHex = function () {
  var s = "";
  for (var i=0;i<this.length;i++)
    s += (256+this[i]).toString(16).substr(-2);
  return s;
}

ArrayBuffer.prototype.toStr = function () {
  return E.toString(this);
}


function fromHex(hex) {
  var arr = new ArrayBuffer(hex.length/2);
  for (var i=0;i<hex.length;i+=2)
    arr[i>>1] = parseInt(hex.substr(i,2),16);
  return arr;
}

function test(a, b) {
  tests++;
  var ar = a();
  if (ar!=b) {
    console.log("Test "+tests,a,"did not equal",b,", it was ", ar);
  } else {
   testPass++;
  }
}

var tests = 0;
var testPass = 0;
test(function() {
  return require('crypto').PBKDF2('Secret Passphrase', fromHex("cbde29d15836ce94e34a124afe1094e2")).toHex();
}, "dd469421e5f4089a1418ea24ba37c61b");


test(function () {
  var key = fromHex("dd469421e5f4089a1418ea24ba37c61b");
  var msg = 'Lots and lots of my lovely secret data          ';
  return require('crypto').AES.encrypt(msg, key).toHex();
}, "b7e812f9a05778e9fb2a09b9edf49e1f12e7543e609fa8bec3d9ab82b5b206f59ff23b5614175f59d66918a1f271e5eb");

test(function () {
  var key = fromHex("dd469421e5f4089a1418ea24ba37c61b");
  var msg = fromHex("b7e812f9a05778e9fb2a09b9edf49e1f12e7543e609fa8bec3d9ab82b5b206f59ff23b5614175f59d66918a1f271e5eb");
  return require('crypto').AES.decrypt(msg, key).toStr();
}, 'Lots and lots of my lovely secret data          ');

var iv = "Hello World 1234"; // 16 bytes (can be an array too)

test(function () {
  var key = fromHex("dd469421e5f4089a1418ea24ba37c61b");
  var msg = 'Lots and lots of my lovely secret data          ';
  return require('crypto').AES.encrypt(msg, key, {iv:iv}).toHex();
}, "66a140b8d735597643d4dfeb1f5b8f23516363e9f7760d6a5bbc8659f0a9bccf7fdd55dfc1fc84945443fdfe877238ed");

test(function () {
  var key = fromHex("dd469421e5f4089a1418ea24ba37c61b");
  var msg = fromHex("66a140b8d735597643d4dfeb1f5b8f23516363e9f7760d6a5bbc8659f0a9bccf7fdd55dfc1fc84945443fdfe877238ed");
  return require('crypto').AES.decrypt(msg, key, {iv:iv}).toStr();
}, 'Lots and lots of my lovely secret data          ');


result = tests==testPass;
