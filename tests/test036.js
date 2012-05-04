// the 'lf' in the printf caused issues writing doubles on some compilers
var a=5.0/10.0*100.0;
var b=5.0*110.0;
var c=50.0/10.0;
a.dump();
b.dump();
c.dump();
result = a==50 && b==550 && c==5;

