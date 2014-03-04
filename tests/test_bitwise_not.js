// Bitwise not

var tests = [
~2 == -3,
~0xFF == -256,
~-3 == 2,
];

result = 1;
for (i in tests)
  if (!tests[i])
    result=0;


