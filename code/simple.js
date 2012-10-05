function foo(a,b) { return a+333*b;};foo(1,4);

function flash(c) {
  if (c<=0) return;
  setTimeout(function() {
    digitalWrite("C9",1);
    setTimeout(function() {
      digitalWrite("C9",0);
      flash(c-1);
    }, 250);
  }, 250);
}

var count = 0;
setWatch(function() {
 if (!digitalRead("A0")) return;
 flash(count++);
}, "A0", true);



var count = 0;                                                                                         
setWatch(function () {                                                                                 
 if (!digitalRead("A0")) return;                                                                       
 print(count);
 count++;
}, "A0", 1);    


