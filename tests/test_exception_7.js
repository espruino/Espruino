// http://forum.espruino.com/conversations/273386/

var  r = "";

function log(s) {
  r += s+"\n";
  console.log(s);
}

function outer() {
 try {
   log('In outer');
   inner();
   log('Out outer');
 } catch(x) {
   log('Catch outer '+x);
 }
}

function inner() {
 try {
   log('In inner');
   throw "coucou";
   log('Out inner');
 } catch(x) {
   log('Catch inner '+x);
   throw x;
 }
}

outer();
result = r=="In outer\nIn inner\nCatch inner coucou\nCatch outer coucou\n";


