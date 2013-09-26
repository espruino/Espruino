// testing replaceWith functionality

var inner = function() {
  var x = 5;
  return function() { return x; }
}();

//trace(inner);
inner.replaceWith(function() { return x+2; });
//trace(inner);

result = inner()==7;
