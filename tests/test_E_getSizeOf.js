var b = [1,2,3];
var a = [b,b,b];
a[3] = a; // test that recursion doesn't mess us up

var sb = E.getSizeOf(b);
var sa = E.getSizeOf(a);

result = sa == (sb + 1 /* array */ + 4 /* indices */);
