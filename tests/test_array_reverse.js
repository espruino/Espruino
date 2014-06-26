a = [1,2,3].reverse();
b = [1,2,3,4].reverse();

c = [];
c[3] = 42;
c.reverse();

d = [];
d[7] = "a";
d[3] = "b";
d.reverse();

av = a.length+":"+a.toString();
bv = b.length+":"+b.toString();
cv = c.length+":"+c.toString();
dv = d.length+":"+d.toString();

result = av=="3:3,2,1" && bv=="4:4,3,2,1" && cv=="4:42,,," && dv=="8:a,,,,b,,,";
