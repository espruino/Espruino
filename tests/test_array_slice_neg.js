// negative numbers int array break iteration

a = [ 1, 2, 3 ]
r1 = a.slice()=="1,2,3";
a[-1]=4;
r2 = a.slice()=="1,2,3";
for (var i of a) console.log(i); // should return 1,2,3

result = r1&&r2;
