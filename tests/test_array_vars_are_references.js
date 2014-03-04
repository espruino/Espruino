// references for arrays

var a = [];
a[0] = 10;
a[1] = 22;

b = a;

b[0] = 5;

result = a[0]==5 && a[1]==22 && b[1]==22;
