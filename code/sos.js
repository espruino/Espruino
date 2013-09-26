var msg = "... --- ...";
var led = "C9";
var button = "A0";
var n = 0;

function char() {
 var c = msg.charAt(n++);
 if (c==".") {
  digitalWrite(led,1);
  setTimeout("digitalWrite(led,0);", 250);
  setTimeout(char, 500);
 } else if (c=="-") {
  digitalWrite(led,1);
  setTimeout("digitalWrite(led,0);", 500);
  setTimeout(char, 750);
 } else if (c==" ") {
  setTimeout(char, 500);
 } // else it's the end of the string - do nothing
}

setWatch("if (digitalRead(button)) {n=0;char();}",button,true);
// if this is started many times, we get a memory leak
