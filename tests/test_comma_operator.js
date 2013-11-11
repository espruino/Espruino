var a = 0, b = 0;
for (i=1,2,3;i<10;i++) { console.log(i); a+=i; }
for (i=(1,2,3);i<10;i++) { console.log(i); b+=i; }
result = a==45 && b==42;
