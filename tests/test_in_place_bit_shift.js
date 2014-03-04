// in-place shift

var a = 1;

var tests = [
a<<=1 == 2,
a<<=5 == 64,
a>>=1 == 32,
a>>>=1 == 16,
];

result = 1;
for (i in tests)
  if (!tests[i])
    result=0;


