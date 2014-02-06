// From TiCPU, http://forum.espruino.com/conversations/568

// Useful functions
Array.prototype.median = function () {
  return this[Math.floor(this.length/2)];
};
Uint16Array.prototype.median = Array.prototype.median;
Array.prototype.swap = function (left, right) {
  var tmp = this[left];
  
  this[left] = this[right];
  this[right] = tmp;
  
  return this;
};
Uint16Array.prototype.swap = Array.prototype.swap;
Uint16Array.prototype.clear = function () {
  for (var i = 0; i < this.length; i++) this[i] = 0;
};
// Bubble sort
Array.prototype.sort = function () {
  var len = this.length - 1;
  for (var i = 0; i < len; i++) {
    for (var j = 0, swapping, endIndex = len - i; j < endIndex; j++) {
      if (this[j] > this[j + 1]) {
        swapping = this[j];
        this[j] = this[j + 1];
        this[j + 1] = swapping;
      }
    }
  }
  return this;
};
Uint16Array.prototype.sort = Array.prototype.sort;

// Tests
var tBsort = new Uint16Array([5454,5449,5380,5412,5380,5366,5344,5395,5398,5424,5422,5473,5420,5432,5376,5354,5561,5288,5393,5388,5422,5427,5476,5407,5385,5180,5363,5324,5395,5393,5410,5405,5349,5361,5385,5412,5373,5373,5478,5420,5446,5395,5339,5407,5420,5356,5336,5427,5459,5378,5336,5349,5420,5405,5434,5383,5446,5422,5349,5329,5405,5434,5446,5336,5427,5473,5402,5170,5388,5412,5456,123,456,789]);

tBsort.sort();
