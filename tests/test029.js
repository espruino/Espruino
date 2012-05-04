// test for array remove
var a = [1,2,4,5,7];

a.remove(2);
a.remove(5);

result = a.length==3 && a[0]==1 && a[1]==4 && a[2]==7;
