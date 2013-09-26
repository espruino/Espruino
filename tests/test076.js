// array splice

Array.prototype.equals = function(arr) {
  if (this.length != arr.length) return false;
  for (var i=0;i<this.length;i++)
    if (this[i]!=arr[i]) return false;
  return true;
}



var fails = 0;
// remove half
var a = [0,1,2,3,4,5];
if (!a.splice(2).equals([2,3,4,5])) fails |= 1;
if (!a.equals([0,1])) fails |= 2;
// remove half from end
var a = [0,1,2,3,4,5];
if (!a.splice(-1).equals([5])) fails |= 4;
if (!a.equals([0,1,2,3,4])) fails |= 8;
// remove variable amount
var a = [0,1,2,3,4,5];
if (!a.splice(3,1).equals([3])) fails |= 16;
if (!a.equals([0,1,2,4,5])) fails |= 32;
// remove none, add some
var a = [0,1,2,3,4,5];
if (!a.splice(-2,0,42,43,44).equals([])) fails |= 64;
if (!a.equals([0,1,2,3,42,43,44,4,5])) fails |= 128;
// remove none, add some at end
var a = [0,1,2,3,4,5];
if (!a.splice(10000,0,42,43,44).equals([])) fails |= 256;
if (!a.equals([0,1,2,3,4,5,42,43,44])) fails |= 512;
// remove some, add some
var a = [0,1,2,3,4,5];
if (!a.splice(1,4,42,43,44).equals([1,2,3,4])) fails |= 1024;
if (!a.equals([0,42,43,44,5])) fails |= 2048;
// remove none, add some at start
var a = [0,1,2,3,4,5];
if (!a.splice(0,0,42,43,44).equals([])) fails |= 4096;
if (!a.equals([42,43,44,0,1,2,3,4,5])) fails |= 8192;

result = fails==0;


