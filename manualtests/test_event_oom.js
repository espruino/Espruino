
// test for out of memory while adding stuff
setInterval(function() {
  var led=20;
  var flasher = setInterval(function() { 
   led--;
   digitalWrite("C9",led&1); 
   if (led<=0) clearInterval(flasher);
  }, 200);
}, 200, 1);
