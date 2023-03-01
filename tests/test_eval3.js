result = 0;
(function(){eval(`var x = 42;
print(x); // access the var that was defined here
result = 1;
`)})()
if (global.x) result=0;
