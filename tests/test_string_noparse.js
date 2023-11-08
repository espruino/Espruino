function fail(n) {
  throw new Error("FAILED at "+n);
}

/* We hit this with minification in the AT library... If there's a string right after an if(0) it
doesn't get parsed! 

This happens because since 2v19ish we're no longer storing string contents in RAM if
we're not supposed to be executing. But we need to be sure we don't Lex too far ahead
with execute=false set
*/

while (0) 1;"test"||fail("while") // ok
for (;0;);"test"||fail("for")     // ok
if (1) 1;"test"||fail("if(1)1;")   // ok
if (0) 1;"test"||fail("if(0)1;")   // failed
if (0);"test"||fail("if(0);")      // failed
if (1);else;"test"||fail("if(1);else;") // failed
(function(){ if(0)return 42;'OK'!='OK'&&fail('if(0)return')})()
// if we got here we're ok
result=1
