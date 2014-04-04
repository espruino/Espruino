var count = 0;
var exprs = [
  !!1, !!"1", !!function() {}, !![], !![1], !![1, 2], !!{a:1}, !!({a:1}), !!true,
  !0, !"", !null, !false, !undefined
];

// Uncomment when Array.prototype.every is implemented
// result = exprs.every(function(expr) {
//   return expr;
// });

for (var i = 0; i < exprs.length; i++) {
  if (exprs[i]) {
    count++;
  }
}
result = exprs.length === count;
