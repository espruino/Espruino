// Random parsing error that we came across...

var A1 = 1;
var A2 = 2;
var A3 = 3;

function bar(a,b) {
}

var a = 0;

function foo() {
 if (a==0) bar([A1, A2, A3], 0b100);
 if (a==1) bar([A1, A2, A3], 0b101);
 if (a==2) bar([A1, A2, A3], 0b001);
 if (a==3) bar([A1, A2, A3], 0b011);
 if (a==4) bar([A1, A2, A3], 0b010);
 if (a==5) bar([A1, A2, A3], 0b110);
 if (a==6) bar([A1, A2, A3], 0b111);
 if (a==7) bar([A1, A2, A3], 0b000);
 result = 1;
}

foo();
