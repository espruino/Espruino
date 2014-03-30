setTimeout(function() {
  clearInterval(); // FIXME this causes an assert fail!!!
  setTimeout(function() {
   result = 1;
  },10);
}, 10);

