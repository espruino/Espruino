// test for array remove
// note: array.remove does not exist in the JavaScript standard
Array.prototype.remove = function(x) { 
  var idx = this.indexOf(x);
  if (idx<0) return; // not in array
  var l = this.length;
  for (var i=idx+1;i<l;i++)
    this[i-1]=this[i];
  this.pop(); // pop off the old value
} 

var a = [1,2,4,5,7];

a.remove(2);
a.remove(5);

result = a.length==3 && a[0]==1 && a[1]==4 && a[2]==7;
