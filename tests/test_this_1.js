// https://github.com/espruino/Espruino/issues/233

String.intervals = [];

String.prototype.foo = function(period) {
  console.log(this);
  String.intervals[this]; // Used to reset 'this' by turning it into a 'name'
  console.log(this);  
  result = this == "Hello";
};

"Hello".foo(100); 
