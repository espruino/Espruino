// Formerly test055.js - added so a search in directory can reveal this file
// test  that setTimeout acts like jsConsole/Chrome's
// HOWEVER this is not always the case. See what is reported on this bug: https://github.com/espruino/Espruino/issues/134
// It does make a lot of sense for Espruino to do this though (see the comments in the bug)

var result=0;
function a() {result=0;}
setTimeout(a,10);
function a() {result=1;}
