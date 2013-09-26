// test  that setTimeout acts like jsConsole/Chrome's

var result=0;
function a() {result=0;}
setTimeout(a,10);
function a() {result=1;}
