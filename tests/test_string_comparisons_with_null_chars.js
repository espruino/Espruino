// String equals with null

var tests = [
"\0" == "\0",
!("\0A" == "\0B"),
"\0A" != "\0B",
];

result = 1;
for (i in tests)
  if (!tests[i])
    result=0;


