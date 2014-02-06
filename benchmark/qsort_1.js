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
// Quick sort
Array.prototype.qsort_partition = function (pivot, left, right) {
  var tmp, storeIndex = left, pivotVal = this[pivot];
  this.swap(pivot, right);
  for (var i = left; i < right; i++) {
    if (this[i] < pivotVal) {
      this.swap(i, storeIndex);
      storeIndex++;
    }
  }
  this.swap(right, storeIndex);
  return storeIndex;
};
Uint16Array.prototype.qsort_partition = Array.prototype.qsort_partition;
Array.prototype.qsort = function (left, right) {
  var pivot, newPivot;
  if (left === undefined) {
    left = 0;
    right = this.length - 1;
  }
  if (left < right) {
    pivot = left + Math.ceil((right - left) / 2);
    newPivot = this.qsort_partition(pivot, left, right);
    this.qsort(left, newPivot - 1);
    this.qsort(newPivot + 1, right);
  }
  return this;
};
Uint16Array.prototype.qsort = Array.prototype.qsort;

// Tests
var tQsort = new Uint16Array([5454,5449,5380,5412,5380,5366,5344,5395,5398,5424,5422,5473,5420,5432,5376,5354,5561,5288,5393,5388,5422,5427,5476,5407,5385,5180,5363,5324,5395,5393,5410,5405,5349,5361,5385,5412,5373,5373,5478,5420,5446,5395,5339,5407,5420,5356,5336,5427,5459,5378,5336,5349,5420,5405,5434,5383,5446,5422,5349,5329,5405,5434,5446,5336,5427,5473,5402,5170,5388,5412,5456,123,456,789]);
tQsort.qsort();
